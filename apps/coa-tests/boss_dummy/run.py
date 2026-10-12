import os
from pathlib import Path
import subprocess
import tempfile

ROOT = Path(__file__).resolve().parents[3]


def method(text, name):
    start = text.rfind('\n', 0, text.index(name)) + 1
    brace = text.index('{', start)
    depth = 1
    end = brace + 1
    while depth:
        depth += (text[end] == '{') - (text[end] == '}')
        end += 1
    return text[start:end]


def main():
    source = (ROOT / 'src/server/scripts/World/npc_advanced_training_dummy.cpp').read_text()
    methods = '\n'.join(method(source, name) for name in (
        'bool UpdateBossSession()', 'void SetBossCounterImmunity(', 'void HandleBossEvent('))
    code = (Path(__file__).parent / 'scenario.cpp.in').read_text().replace('ACTUAL_METHODS', methods)
    with tempfile.TemporaryDirectory() as directory:
        path = Path(directory)
        (path / 'test.cpp').write_text(code)
        subprocess.run([os.environ.get('CXX', 'c++'), '-std=c++20', str(path / 'test.cpp'),
                        '-o', str(path / 'test')], check=True)
        subprocess.run([str(path / 'test')], check=True)
    print('PASS: boss rotation, counter windows, lockout, range, reset and melee modifier restoration')


if __name__ == '__main__':
    main()
