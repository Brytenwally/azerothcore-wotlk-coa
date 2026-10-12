from pathlib import Path
import os
import runpy
import shutil
import subprocess
import tempfile

HERE = Path(__file__).resolve().parent
ROOT = HERE.parents[2]

def main():
    method = runpy.run_path(str(HERE.parent / 'client_compat/run.py'))['method']
    source = (ROOT / 'src/server/coa/AscensionCompat.cpp').read_text(encoding='utf-8')
    bodies = '\n'.join(method(source, signature) for signature in (
        'uint32 GetIncarnationDisplay(', 'uint32 GetIncarnationSpell(',
        'void RefreshCosmetics(', 'void RefreshIncarnation(', 'void RefreshIncarnations('))
    harness = (HERE / 'harness.cpp').read_text(encoding='utf-8-sig').replace('// METHODS', bodies)
    compiler = os.environ.get('CXX') or shutil.which('c++') or shutil.which('g++')
    if not compiler:
        raise RuntimeError('C++ compiler is required')
    with tempfile.TemporaryDirectory(prefix='vanity-incarnations-') as temp:
        out = Path(temp)
        (out / 'harness.cpp').write_text(harness, encoding='utf-8')
        exe = out / 'regressions'
        subprocess.run([compiler, '-std=c++20', '-Wall', '-Wextra', '-Werror',
                        '-I' + str(ROOT / 'src/server/coa'), str(out / 'harness.cpp'), '-o', str(exe)], check=True)
        subprocess.run([str(exe)], check=True)
    print('Native incarnation methods: forms, pet families, ownership, category isolation, transforms, visibility, restoration passed')

if __name__ == '__main__':
    main()
