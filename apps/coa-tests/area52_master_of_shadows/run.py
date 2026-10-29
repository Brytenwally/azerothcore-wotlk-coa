import os
from pathlib import Path
import shutil
import subprocess
import tempfile

HERE = Path(__file__).resolve().parent
ROOT = HERE.parents[2]

HARNESS = r'''#include "Area52MasterOfShadowsRules.h"
#include <cassert>
using namespace Area52MasterOfShadows;

int main()
{
    SpellKey const sinister{1752, 8, 0x800002};
    SpellKey const flurry{965304, 8, 0x800002};
    SpellKey const eviscerate{2098, 8, 0x820000};
    SpellKey const rupture{1943, 8, 0x100000};
    SpellKey const garrote{703, 8, 0x100};
    SpellKey const gloomblade{760416, 8, 0};
    SpellKey const tempest{254902, 0, 0};
    SpellKey const darkIntent{980050, 8, 0x2};
    SpellKey const shadowBolt{686, 5, 0x1};
    assert(TriggersDarkIntent(sinister, false) && TriggersDarkIntent(flurry, false));
    assert(TriggersDarkIntent(garrote, true) && TriggersDarkIntent(rupture, true) && TriggersDarkIntent(tempest, true));
    assert(TriggersDarkIntent(gloomblade, false) && !TriggersDarkIntent(gloomblade, true));
    assert(!TriggersDarkIntent(darkIntent, false) && !TriggersDarkIntent(shadowBolt, false));
    assert(GrantsApprentice(sinister, false, true) && GrantsApprentice(flurry, false, true));
    assert(GrantsApprentice(gloomblade, false, true) && !GrantsApprentice(sinister, false, false));
    assert(!GrantsApprentice(eviscerate, false, true) && !GrantsApprentice(gloomblade, true, true));
    assert(SpendsApprentice(eviscerate, false, 3) && SpendsApprentice(rupture, false, 3));
    assert(!SpendsApprentice(eviscerate, false, 2) && !SpendsApprentice(rupture, true, 3));
    assert(!SpendsApprentice(sinister, false, 3) && IsShadowBolt(shadowBolt) && !IsShadowBolt(sinister));
    assert(ShouldCopyRemaining(9000, 0, false) && ShouldCopyRemaining(9000, 3000, true));
    assert(!ShouldCopyRemaining(9000, 12000, true) && !ShouldCopyRemaining(0, 0, false));
    assert(PestilenceRadius == 10.0f && PestilenceMaxTargets == 6 && NightfallDarkIntentHits == 2);
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
    enchant = (ROOT / 'src/server/coa/AscensionMysticEnchant.cpp').read_text(encoding='utf-8')
    assert 'HasArea52MasterOfShadowsHandlers(spell)' in enchant
    assert 'new area52_master_of_shadows_casts();' in enchant
    sql = (ROOT / 'data/sql/updates/pending_db_world/rev_20261029_20_master_of_shadows_build.sql').read_text(
        encoding='utf-8')
    for row in ('(980046, 0, 0, 0, 0, 0, 262160, 7, 2, 0, 2, 2, 0, 100, 0, 0)',
                '(81182, 0, 8, 2, 0, 0, 16, 1, 2, 0, 0, 0, 0, 100, 0, 0)',
                '(979750, 0, 0, 0, 0, 0, 16, 1, 2, 0, 0, 0, 0, 50, 5000, 0)',
                "(980050, 0.115, 0, 0.115, 0, 'Area 52 Dark Intent')",
                "(990248, 0.115, 0, 0.115, 0, 'Area 52 Dark Intent')",
                "(980046, 'spell_area52_master_of_shadows')", "(980048, 'spell_area52_apprentice_of_shadows')",
                "(81182, 'spell_area52_pestilence')", "(931234, 'spell_area52_find_weakness')"):
        assert row in sql, row
    with tempfile.TemporaryDirectory(prefix='coa-area52-master-of-shadows-') as directory:
        out = Path(directory)
        cpp, exe = out / 'shadows.cpp', out / 'shadows.exe'
        cpp.write_text(HARNESS, encoding='utf-8')
        subprocess.run(command(cpp, exe), cwd=out, check=True, timeout=60)
        subprocess.run([str(exe)], cwd=out, check=True, timeout=15)
    print('area52 master of shadows rules: ok')


if __name__ == '__main__':
    main()
