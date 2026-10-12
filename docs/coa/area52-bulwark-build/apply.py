import argparse
import hashlib
import json
from pathlib import Path
import subprocess


def digest(path):
    return hashlib.sha256(path.read_text(encoding='utf-8').encode('utf-8')).hexdigest() if path.exists() else None


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('integration_source', type=Path)
    parser.add_argument('--check', action='store_true')
    args = parser.parse_args()
    package = Path(__file__).resolve().parent
    root = args.integration_source.resolve(strict=True)
    files = json.loads((package / 'source-preconditions.json').read_text())['files']
    actual = {}
    for relative in files:
        target = (root / relative).resolve()
        if not target.is_relative_to(root):
            raise ValueError('Source path escapes checkout')
        actual[relative] = digest(target)
    if all(actual[name] == record['after'] for name, record in files.items()):
        print('Already applied')
        return
    mismatches = [name for name, record in files.items() if actual[name] != record['before']]
    if mismatches:
        raise SystemExit('Reconcile source before applying: ' + ', '.join(mismatches))
    patch = str(package / 'runtime-repairs.patch')
    subprocess.run(['git', '-C', str(root), 'apply', '--check', patch], check=True)
    if args.check:
        print('All source preconditions and patch checks passed')
        return
    subprocess.run(['git', '-C', str(root), 'apply', patch], check=True)
    for name, record in files.items():
        if digest(root / name) != record['after']:
            raise RuntimeError('Post-apply source mismatch: ' + name)
    print('Applied and verified all source changes')


if __name__ == '__main__':
    main()
