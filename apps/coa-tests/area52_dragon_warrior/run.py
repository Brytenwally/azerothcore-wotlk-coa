import os
from pathlib import Path
import shutil
import subprocess
import tempfile

HERE = Path(__file__).resolve().parent
ROOT = HERE.parents[2]

HARNESS = r'''#include "Area52DragonWarriorRules.h"
#include <cassert>
using namespace Area52DragonWarrior;

int main()
{
    assert(!RoarStuns(0) && !RoarStuns(1) && !RoarStuns(2));
    assert(RoarStuns(3) && RoarStuns(5));
    assert(SlamAlwaysCrits(true, false) && SlamAlwaysCrits(false, true) && SlamAlwaysCrits(true, true));
    assert(!SlamAlwaysCrits(false, false));
    assert(FuryCooldownCutMs == 5000 && SlamFamilyMask0 == 0x200000);
    return 0;
}
'''


def command(source, exe):
    if os.environ.get('VCToolsInstallDir'):
        compiler = Path(os.environ['VCToolsInstallDir']) / 'bin/Hostx64/x64/cl.exe'
        return [str(compiler), '/nologo', '/std:c++20', '/EHsc', '/W4', '/WX', '/I', str(ROOT / 'src/server/coa'),
                str(source), '/Fe' + str(exe)]
    compiler = shutil.which(os.environ.get('CXX', 'g++'))
    assert compiler, 'Set CXX to a C++17 compiler or run from an MSVC developer environment.'
    return [compiler, '-std=c++17', '-Wall', '-Wextra', '-Werror', '-I', str(ROOT / 'src/server/coa'), str(source),
            '-o', str(exe)]


def main():
    grants = (ROOT / 'src/server/coa/AscensionMysticSpellGrants.h').read_text(encoding='utf-8')
    assert '{954252, 954250, 12292, 12292}' in grants
    sql = (ROOT / 'data/sql/updates/pending_db_world/rev_20261028_20_dragon_warrior_build.sql').read_text(
        encoding='utf-8')
    for row in ("(954253, 'spell_area52_dragon_fury')", "(954251, 'spell_area52_dragon_roar_stun')",
                '(954261, 0, 4, 2097152, 0, 0, 16, 7, 2, 0, 0, 0, 0, 100, 0, 1)'):
        assert row in sql, row
    with tempfile.TemporaryDirectory(prefix='coa-area52-dragon-warrior-') as directory:
        out = Path(directory)
        cpp, exe = out / 'dragon_warrior.cpp', out / 'dragon_warrior.exe'
        cpp.write_text(HARNESS, encoding='utf-8')
        subprocess.run(command(cpp, exe), cwd=out, check=True, timeout=60)
        subprocess.run([str(exe)], cwd=out, check=True, timeout=15)
    print('area52 dragon warrior rules: ok')


if __name__ == '__main__':
    main()
