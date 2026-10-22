from __future__ import annotations

CLI_DESCRIPTION = """Deliver in-game reports straight to GitHub issues of one repository. Standard library only.

Runs the same spool, journal and lock as relay.py, but posts with a GitHub token of the server owner instead of
the Railway service. Dry-run is the default. --send reads the token from COA_BUGREPORT_GITHUB_TOKEN.
Never run two instances, or this and relay.py, against the same spool; the CLI enforces an OS file lock.
"""

import argparse
import importlib.util
import json
import os
from pathlib import Path
import re
import sys
import time
import urllib.error
import urllib.request

HERE = Path(__file__).resolve().parent
_spec = importlib.util.spec_from_file_location("coa_bug_relay", HERE / "relay.py")
relay = importlib.util.module_from_spec(_spec)
_spec.loader.exec_module(relay)

DEFAULT_REPOSITORY = "Brytenwally/azerothcore-wotlk-coa"
TOKEN_VARIABLE = "COA_BUGREPORT_GITHUB_TOKEN"
REPOSITORY_VARIABLE = "COA_BUGREPORT_REPOSITORY"
REPOSITORY = re.compile(r"[A-Za-z0-9-]{1,39}/[A-Za-z0-9._-]{1,100}")
MENTION = re.compile(r"@(?=[A-Za-z0-9])")
GITHUB_API = "https://api.github.com"


def neutralize_mentions(text: str) -> str:
    return MENTION.sub("@​", text)


class GitHubIssues:
    def __init__(self, repository: str, token: str):
        if not REPOSITORY.fullmatch(repository or ""):
            raise ValueError("Configure the target repository as owner/name.")
        if not token or not token.isascii() or any(c.isspace() for c in token):
            raise ValueError(f"Set {TOKEN_VARIABLE} to a GitHub token with issue write access.")
        self.repository = repository
        self.token = token
        self.api = f"{GITHUB_API}/repos/{repository}/issues"
        self.web = f"https://github.com/{repository}/issues/"
        self.opener = urllib.request.build_opener(relay.NoRedirect())

    @staticmethod
    def limit_delay(headers, default=300):
        if headers is None:
            return default
        if headers.get("Retry-After"):
            return relay.ReportService.retry_delay(headers, default)
        if headers.get("X-RateLimit-Remaining") == "0":
            try:
                return max(1, min(int(headers.get("X-RateLimit-Reset", "0")) - int(time.time()), 86400))
            except (ValueError, TypeError):
                return default
        return None

    def create(self, title: str, body: str) -> int:
        request = urllib.request.Request(
            self.api,
            data=json.dumps({"title": neutralize_mentions(title), "body": neutralize_mentions(body)}).encode("utf-8"),
            method="POST",
            headers={
                "Authorization": "Bearer " + self.token,
                "Accept": "application/vnd.github+json",
                "Content-Type": "application/json",
                "User-Agent": "CoA-InGame-BugReport/1",
                "X-GitHub-Api-Version": "2022-11-28",
            },
        )
        try:
            with self.opener.open(request, timeout=30) as response:
                if response.status != 201:
                    raise relay.DeliveryError("uncertain")
                return self.issue_number(relay.ReportService.read_json(response))
        except urllib.error.HTTPError as error:
            if error.code == 429:
                raise relay.DeliveryError("limited", self.limit_delay(error.headers) or 300) from None
            if error.code == 403:
                delay = self.limit_delay(error.headers)
                if delay is not None:
                    raise relay.DeliveryError("limited", delay) from None
                raise relay.DeliveryError("blocked") from None
            if error.code in (400, 413, 415, 422):
                raise relay.DeliveryError("invalid") from None
            if error.code in (301, 302, 303, 307, 308, 401, 404, 405, 410):
                raise relay.DeliveryError("blocked") from None
            raise relay.DeliveryError("uncertain") from None
        except (urllib.error.URLError, TimeoutError, OSError, ValueError):
            raise relay.DeliveryError("uncertain") from None

    def issue_number(self, issue):
        if not isinstance(issue, dict):
            raise relay.DeliveryError("uncertain")
        number = issue.get("number")
        if (type(number) is not int or not 0 < number <= 4294967295
                or issue.get("html_url") != self.web + str(number)):
            raise relay.DeliveryError("uncertain")
        return number


def main(argv=None):
    parser = argparse.ArgumentParser(description=CLI_DESCRIPTION)
    parser.add_argument("--spool", type=Path, required=True)
    parser.add_argument("--repository", default=os.environ.get(REPOSITORY_VARIABLE, DEFAULT_REPOSITORY),
                        help="Target repository as owner/name.")
    parser.add_argument("--once", action="store_true", help="Process one pass, then exit.")
    modes = parser.add_mutually_exclusive_group()
    modes.add_argument("--send", action="store_true", help="Enable real issue creation on GitHub.")
    modes.add_argument("--dry-run", action="store_true", help="Validate files without writes or network (default).")
    args = parser.parse_args(argv)
    root = args.spool.resolve(strict=True)
    if not root.is_dir():
        parser.error("--spool must be an existing private directory")
    if not REPOSITORY.fullmatch(args.repository):
        parser.error("--repository must be owner/name")
    os.umask(0o077)
    if not args.send:
        count = 0
        for path in sorted(root.glob("*.report")):
            relay.read_report(path)
            count += 1
        print(f"Dry run: {count} report(s) valid for {args.repository}; no network requests or file writes.")
        return 0
    service = GitHubIssues(args.repository, os.environ.get(TOKEN_VARIABLE, ""))
    with relay.worker_lock(root):
        worker = relay.Relay(root, service)
        try:
            while True:
                for path in sorted(root.glob("*.report")):
                    worker.process(path)
                if args.once:
                    break
                time.sleep(5)
        finally:
            worker.close()
    return 0


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except KeyboardInterrupt:
        raise SystemExit(0)
    except (OSError, ValueError, RuntimeError, relay.sqlite3.Error) as error:
        reason = str(error) if isinstance(error, ValueError) else type(error).__name__
        print(f"Relay stopped ({reason}). Check configuration, permissions and the delivery journal.", file=sys.stderr)
        raise SystemExit(1)
