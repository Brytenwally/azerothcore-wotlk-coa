import os
from pathlib import Path
import shutil
import subprocess
import tempfile

HERE = Path(__file__).resolve().parent
ROOT = HERE.parents[2]

HARNESS = r'''#include "Area52TreeOfWrathRules.h"
#include <cassert>
using namespace Area52TreeOfWrath;

int main()
{
    assert(CooldownAfterCut(15000, LivingLightningCooldownCutMs) == 13000);
    assert(CooldownAfterCut(2000, LivingLightningCooldownCutMs) == 0);
    assert(CooldownAfterCut(500, LivingLightningCooldownCutMs) == 0);
    assert(IsLivingLightningDamage(86449) && IsLivingLightningDamage(86458) && !IsLivingLightningDamage(86452));
    assert(IsLightningOverload(45284) && IsLightningOverload(45297) && !IsLightningOverload(403));
    assert(ChargedTidesStacksFor(false, false) == 1 && ChargedTidesStacksFor(true, false) == 0);
    assert(ChargedTidesStacksFor(true, true) == 1);
    assert(LivingLightningCooldownCutFlags0 == 0x1C0 && ChargedTidesSpenderFlags0 == 0x140);
    assert(TreeOfWrathLightningFlags0 == 0x3 && TranscendentalShamanFlags0 == 0xC0);
    assert(TranscendentalDruidFlags0 == 0x60 && LightningBoltCooldownMs == 6000);
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
    for grant in ('{86438, 86435, 33891, 33891}', '{86438, 86441, 1064, 1064}'):
        assert grant in grants, grant
    player = (ROOT / 'src/server/game/Entities/Player/Player.cpp').read_text(encoding='utf-8')
    form = player.index('originalInfo->HasAura(SPELL_AURA_MOD_SHAPESHIFT)')
    assert form < player.index('WorldPacket packet(SMSG_SUPERCEDED_SPELL, 8);')
    assert 'if (previous == original)\n            SendLearnPacket(original, false);' in player[form:]
    defines = (ROOT / 'src/server/game/Entities/Unit/UnitDefines.h').read_text(encoding='utf-8')
    assert 'FORM_TREE_OF_WRATH                  = 0x30,' in defines
    tree = (ROOT / 'src/server/coa/AscensionTreeOfLife.cpp').read_text(encoding='utf-8')
    assert '(form == FORM_TREE_OF_WRATH && IsTreeOfWrathLightning(spell))' in tree
    shaman = (ROOT / 'src/server/scripts/Spells/spell_shaman.cpp').read_text(encoding='utf-8')
    assert 'caster->GetAuraEffect(SPELL_SHAMAN_BLESSING_OF_THE_ETERNALS_ME, EFFECT_1)' in shaman
    enchant = (ROOT / 'src/server/coa/AscensionMysticEnchant.cpp').read_text(encoding='utf-8')
    assert 'HasArea52TreeOfWrathHandlers(spell)' in enchant
    assert 'new area52_tree_of_wrath_casts();' in enchant
    sql = (ROOT / 'data/sql/updates/pending_db_world/rev_20261029_10_tree_of_wrath_build.sql').read_text(
        encoding='utf-8')
    for row in ('(86441, 86441, 1)', '(86441, 86448, 8)',
                '(86436, 0, 0, 0, 0, 0, 81920, 3, 2, 0, 0, 0, 0, 100, 0, 0)',
                '(983470, 0, 11, 448, 0, 16, 16384, 2, 2, 2, 0, 0, 0, 100, 1000, 0)',
                "(86435, 'spell_area52_tree_of_wrath_form')", "(86436, 'spell_area52_tree_of_wrath')",
                "(86449, 'spell_area52_living_lightning_rod')", "(86458, 'spell_area52_living_lightning_rod')",
                "(911127, 'spell_area52_transcendental_embrace')", "(411127, 'spell_area52_transcendental_jump')",
                "(-86441, 'spell_sha_chain_heal')"):
        assert row in sql, row
    with tempfile.TemporaryDirectory(prefix='coa-area52-tree-of-wrath-') as directory:
        out = Path(directory)
        cpp, exe = out / 'tree.cpp', out / 'tree.exe'
        cpp.write_text(HARNESS, encoding='utf-8')
        subprocess.run(command(cpp, exe), cwd=out, check=True, timeout=60)
        subprocess.run([str(exe)], cwd=out, check=True, timeout=15)
    print('area52 tree of wrath rules: ok')


if __name__ == '__main__':
    main()
