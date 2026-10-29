import os
from pathlib import Path
import shutil
import subprocess
import tempfile

HERE = Path(__file__).resolve().parent
ROOT = HERE.parents[2]

HARNESS = r'''#include "Area52GutripperRules.h"
#include <cassert>
using namespace Area52Gutripper;

int main()
{
    assert(DisembowelPowerBonus(0, 2000.0f) == 0);
    assert(DisembowelPowerBonus(1, 1000.0f) == 17 && DisembowelPowerBonus(5, 1000.0f) == 440);
    assert(DeadlyComboChance(0) == 0 && DeadlyComboChance(1) == 20 && DeadlyComboChance(5) == 100);
    assert(DeadlyComboChance(7) == 100);
    assert(WithExecuteBonus(1000, 9) == 1090 && WithExecuteBonus(1000, 0) == 1000);
    assert(SuddenDeathMaxRank == 29724 && SuddenDeathCooldown == 411167 && Disembowel == 947470);
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
    for grant in ('{977906, 947470, 2098}', '{977906, 977908}', '{982965, 982966, 12323}'):
        assert grant in grants, grant
    enchant = (ROOT / 'src/server/coa/AscensionMysticEnchant.cpp').read_text(encoding='utf-8')
    assert 'if (effect.IsAura() && effect.ApplyAuraName)' in enchant
    sql = (ROOT / 'data/sql/updates/pending_db_world/rev_20261028_40_gutripper_build.sql').read_text(encoding='utf-8')
    for row in ('(947470, 947481, 12)', '(982966, 983158, 8)',
                '(977906, 0, 8, 0, 6, 0, 16, 1, 2, 0, 0, 0, 0, 100, 0, 0)',
                '(977908, 0, 4, 536870912, 0, 0, 16, 1, 2, 0, 0, 0, 0, 100, 0, 0)',
                '(84447, 0, 10, 8388608, 0, 0, 65552, 1, 2, 0, 0, 0, 0, 100, 0, 0)',
                '(782820, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 100, 0, 0)',
                "(982966, 0.5, 0, 0.5, 0, 'Area 52 Piercing Cleaver')",
                "(-947470, 'spell_area52_disembowel')", "(977906, 'spell_area52_blood_and_guts')",
                "(977908, 'spell_area52_executing_disembowel')", "(977909, 'spell_area52_executing_disembowel_burst')",
                "(20647, 'spell_area52_gutripper_execute')", "(-29723, 'spell_area52_sudden_death_cooldown')",
                "(782820, 'spell_area52_necrotic_execute')"):
        assert row in sql, row
    with tempfile.TemporaryDirectory(prefix='coa-area52-gutripper-') as directory:
        out = Path(directory)
        cpp, exe = out / 'gutripper.cpp', out / 'gutripper.exe'
        cpp.write_text(HARNESS, encoding='utf-8')
        subprocess.run(command(cpp, exe), cwd=out, check=True, timeout=60)
        subprocess.run([str(exe)], cwd=out, check=True, timeout=15)
    print('area52 gutripper rules: ok')


if __name__ == '__main__':
    main()
