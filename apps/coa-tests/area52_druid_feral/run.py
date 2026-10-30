import os
from pathlib import Path
import shutil
import subprocess
import tempfile

HERE = Path(__file__).resolve().parent
ROOT = HERE.parents[2]

HARNESS = r'''#include "Area52DruidTalentRules.h"
#include <cassert>
using namespace Area52DruidTalents;

int main()
{
    assert(FeralAggressionAttackPower(60, 150) == 90 && FeralAggressionAttackPower(10, 50) == 5);
    assert(MissingManaShare(1000, 400, 3) == 18 && MissingManaShare(1000, 1000, 3) == 0);
    assert(FeralDamagePct(true, true, false, 6) == 3 && FeralDamagePct(true, false, true, 6) == 6);
    assert(FeralDamagePct(false, true, true, 4) == 4 && FeralDamagePct(false, false, false, 6) == 0);
    assert((CatFormFlag0 | BearFormFlag0) == 0xC0000000u && KingOfTheJungleGcdMs == -500);
    assert(ImprovedMangleCooldownMs == 3000 && PrimalGoreCapstone == 863504);
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
    auras = (ROOT / 'src/server/game/Spells/Auras/SpellAuraEffects.cpp').read_text(encoding='utf-8')
    assert 'player->HasTalent(17007, player->GetActiveSpec()) || player->HasSpell(17007)' in auras
    assert 'if (HotWSpellId && !AscensionFreePick::Applies(player))' in auras
    assert 'AscensionFreePick::Applies(player) ? nullptr :' in auras
    linked = (ROOT / 'src/server/coa/AscensionFreePickLinkedSpells.h').read_text(encoding='utf-8')
    assert '{49377, {16979, 49376}},' in linked
    corrections = (ROOT / 'src/server/game/Spells/SpellInfoCorrections.cpp').read_text(encoding='utf-8')
    aggression = corrections.index('ApplySpellFix({ 16858, 16859, 16860 }')
    assert 'ApplyAuraName = SPELL_AURA_MOD_ATTACK_POWER;' in corrections[aggression:aggression + 200]
    loader = (ROOT / 'src/server/coa/CoAScriptLoader.cpp').read_text(encoding='utf-8')
    assert '    AddSC_Area52DruidFeralTalents();' in loader
    feral = (ROOT / 'src/server/coa/Area52DruidFeralTalents.cpp').read_text(encoding='utf-8')
    for registration in ('RegisterSpellScript(aura_area52_savage_defense_talent);',
                         'RegisterSpellScript(aura_area52_feral_aggression);', 'new area52_feral_aggression_level();',
                         'new area52_feral_damage();', 'new area52_druid_feral_capstones();',
                         'RegisterSpellScript(aura_area52_king_of_the_jungle);',
                         'RegisterSpellScript(aura_area52_elder_hide);',
                         'RegisterSpellScript(spell_area52_dreamstate_mana);',
                         'RegisterSpellScript(aura_area52_earth_and_moon);'):
        assert registration in feral, registration
    sql = (ROOT / 'data/sql/updates/pending_db_world/rev_20261030_30_druid_feral_talents.sql').read_text(
        encoding='utf-8')
    for row in ('(277401, 0, 0, 0, 0, 0, 0, 1, 2, 2, 0, 0, 0, 0, 0, 0)',
                '(16949, 0, 7, 65536, 1048576, 1024, 16, 1, 2, 0, 0, 0, 0, 0, 0, 0)',
                'SET `SpellFamilyMask1` = 1049664, `SpellFamilyMask2` = 1024, `Chance` = 50 WHERE `SpellId` = -48483;',
                '(954198, 954200, 3)', "(-16858, 'aura_area52_feral_aggression')",
                "(48495, 'aura_area52_king_of_the_jungle')", "(16931, 'aura_area52_elder_hide')",
                "(833956, 'spell_area52_dreamstate_mana')", "(60433, 'aura_area52_earth_and_moon')"):
        assert row in sql, row
    with tempfile.TemporaryDirectory(prefix='coa-area52-druid-feral-') as directory:
        out = Path(directory)
        cpp, exe = out / 'feral.cpp', out / 'feral.exe'
        cpp.write_text(HARNESS, encoding='utf-8')
        subprocess.run(command(cpp, exe), cwd=out, check=True, timeout=60)
        subprocess.run([str(exe)], cwd=out, check=True, timeout=15)
    print('area52 druid feral talents: ok')


if __name__ == '__main__':
    main()
