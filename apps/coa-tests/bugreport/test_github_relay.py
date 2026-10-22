import email.message
import importlib.util
import io
import json
from pathlib import Path
import tempfile
import unittest
from unittest.mock import patch
import urllib.error

ROOT = Path(__file__).resolve().parents[3]
spec = importlib.util.spec_from_file_location("coa_github_relay", ROOT / "apps/coa-bugreport/github_relay.py")
github_relay = importlib.util.module_from_spec(spec)
spec.loader.exec_module(github_relay)
relay = github_relay.relay
REPOSITORY = "Brytenwally/azerothcore-wotlk-coa"
ID = "1-" + "b" * 32


class Response(io.BytesIO):
    def __init__(self, status, data):
        super().__init__(json.dumps(data).encode("utf-8"))
        self.status = status
        self.headers = email.message.Message()

    def __enter__(self):
        return self

    def __exit__(self, *exc):
        self.close()


def http_error(code, headers=None):
    message = email.message.Message()
    for key, value in (headers or {}).items():
        message[key] = value
    return urllib.error.HTTPError(github_relay.GITHUB_API, code, "error", message, io.BytesIO(b"{}"))


def created(number):
    return Response(201, {"number": number, "html_url": f"https://github.com/{REPOSITORY}/issues/{number}"})


class GitHubIssuesTests(unittest.TestCase):
    def setUp(self):
        self.service = github_relay.GitHubIssues(REPOSITORY, "github_pat_test")

    def post(self, outcome):
        requests = []

        def opener(request, timeout):
            requests.append(request)
            if isinstance(outcome, Exception):
                raise outcome
            return outcome

        with patch.object(self.service.opener, "open", side_effect=opener):
            try:
                return self.service.create("Ability @fails", "Ping @someone\nExpected: damage"), requests
            except relay.DeliveryError as error:
                return error.kind, requests

    def test_created_issue_returns_its_number(self):
        number, requests = self.post(created(41))
        self.assertEqual(number, 41)
        request = requests[0]
        self.assertEqual(request.full_url, f"https://api.github.com/repos/{REPOSITORY}/issues")
        self.assertEqual(request.get_header("Authorization"), "Bearer github_pat_test")
        payload = json.loads(request.data)
        self.assertEqual(set(payload), {"title", "body"})
        self.assertNotIn("@s", payload["body"])
        self.assertNotIn("@f", payload["title"])

    def test_issue_in_another_repository_is_uncertain(self):
        response = Response(201, {"number": 5, "html_url": "https://github.com/other/repo/issues/5"})
        self.assertEqual(self.post(response)[0], "uncertain")

    def test_failures_map_to_relay_states(self):
        cases = {
            401: "blocked", 404: "blocked", 410: "blocked", 403: "blocked",
            422: "invalid", 429: "limited", 500: "uncertain", 502: "uncertain",
        }
        for code, kind in cases.items():
            with self.subTest(code=code):
                self.assertEqual(self.post(http_error(code))[0], kind)

    def test_secondary_rate_limit_waits(self):
        error = http_error(403, {"Retry-After": "60"})
        with patch.object(self.service.opener, "open", side_effect=error):
            with self.assertRaises(relay.DeliveryError) as raised:
                self.service.create("Title", "Body")
        self.assertEqual((raised.exception.kind, raised.exception.delay), ("limited", 60))

    def test_network_failure_is_uncertain(self):
        self.assertEqual(self.post(urllib.error.URLError("down"))[0], "uncertain")

    def test_configuration_is_checked(self):
        with self.assertRaises(ValueError):
            github_relay.GitHubIssues("not a repository", "token")
        with self.assertRaises(ValueError):
            github_relay.GitHubIssues(REPOSITORY, "")


class RelayWithGitHubTests(unittest.TestCase):
    def test_report_becomes_one_issue(self):
        with tempfile.TemporaryDirectory(prefix="coa-github-relay-test-") as name:
            root = Path(name)
            path = root / (ID + ".report")
            path.write_bytes(b"COABUG1\nQuest stuck\nExpected: credit\nActual: none\n")
            service = github_relay.GitHubIssues(REPOSITORY, "github_pat_test")
            worker = relay.Relay(root, service, lambda: 10000)
            try:
                with patch.object(service.opener, "open", side_effect=lambda request, timeout: created(7)) as opened:
                    self.assertEqual(worker.process(path), "created")
                    worker.process(path)
                self.assertEqual(opened.call_count, 1)
                self.assertEqual(path.with_suffix(".status").read_text().strip(), "created|7")
            finally:
                worker.close()


if __name__ == "__main__":
    unittest.main()
