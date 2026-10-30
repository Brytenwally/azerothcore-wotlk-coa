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
    for (std::uint32_t reach : NaturesReach)
    {
        assert(ModifierTarget(reach, 2912, 2912, true).value_or(false));
        assert(!ModifierTarget(reach, 774, 774, false).value_or(true));
    }
    for (std::uint32_t moonglow : Moonglow)
    {
        assert(ModifierTarget(moonglow, MoonglowBuff, MoonglowBuff, false).value_or(false));
        assert(!ModifierTarget(moonglow, 16814, 16814, false).value_or(true));
    }
    assert(!ModifierTarget(16814, 2912, 2912, true).has_value());
    assert(ModifierTarget(SplendorFlourishAndInsectSwarm, 760174, 760174, false).value_or(false));
    assert(ModifierTarget(SplendorFlourishAndInsectSwarm, 24977, InsectSwarm, false).value_or(false));
    assert(!ModifierTarget(SplendorFlourishAndInsectSwarm, 8921, Moonfire, false).value_or(true));
    assert(VengefulSoulMana(1000, 400) == 60 && VengefulSoulMana(1000, 1000) == 0 && VengefulSoulMana(1000, 1200) == 0);
    assert(ShouldSpreadInsectSwarm(8000, 0, false) && ShouldSpreadInsectSwarm(8000, 3000, true));
    assert(!ShouldSpreadInsectSwarm(8000, 9000, true));
    assert(VengeanceCritBonusPct(0) == 0 && VengeanceCritBonusPct(1) == 60 && VengeanceCritBonusPct(5) == 100);
    assert(50 + 50 * VengeanceCritBonusPct(1) / 100 == 80 && 50 + 50 * VengeanceCritBonusPct(5) / 100 == 100);
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
    info = (ROOT / 'src/server/game/Spells/SpellInfo.cpp').read_text(encoding='utf-8')
    assert 'Area52DruidTalents::ModifierTarget(mod->spellId, Id, firstRank, castDamageSpell)' in info
    corrections = (ROOT / 'src/server/game/Spells/SpellInfoCorrections.cpp').read_text(encoding='utf-8')
    reach = corrections.index('ApplySpellFix({ 16819, 16820, 816819 }')
    assert 'MiscValue = SPELLMOD_RANGE;' in corrections[reach:reach + 200]
    unit = (ROOT / 'src/server/game/Entities/Unit/Unit.cpp').read_text(encoding='utf-8')
    assert 'GetAuraEffectOfRankedSpell(16836, 0)' not in unit
    assert unit.index('GetArea52TalentCritDamageBonusPct(caster, spellProto)') < unit.index(
        'modOwner->ApplySpellMod(spellProto->Id, SPELLMOD_CRIT_DAMAGE_BONUS, crit_bonus);')
    druid = (ROOT / 'src/server/scripts/Spells/spell_druid.cpp').read_text(encoding='utf-8')
    assert 'amount += 5 * brambles->GetSpellInfo()->GetRank();' in druid
    assert 'GetAuraEffect(SPELL_AURA_ADD_FLAT_MODIFIER, SPELLFAMILY_DRUID, 53, 2)' not in druid
    eclipse = (ROOT / 'src/server/coa/Area52KnightOfTheEclipse.inc').read_text(encoding='utf-8')
    assert 'GetAuraEffectOfRankedSpell(Area52DruidTalents::CelestialBane, EFFECT_0)' in eclipse
    procs = (ROOT / 'src/server/coa/AscensionTalentDataProcs.cpp').read_text(encoding='utf-8')
    assert 'RegisterSpellScript(aura_ascension_celestial_focus);' in procs
    sql = (ROOT / 'data/sql/updates/pending_db_world/rev_20261030_10_druid_balance_talents.sql').read_text(
        encoding='utf-8')
    loader = (ROOT / 'src/server/coa/CoAScriptLoader.cpp').read_text(encoding='utf-8')
    assert '    AddSC_Area52DruidBalanceCapstones();' in loader
    capstones = (ROOT / 'src/server/coa/Area52DruidBalanceCapstones.cpp').read_text(encoding='utf-8')
    for registration in ('new area52_druid_balance_capstones();', 'RegisterSpellScript(aura_area52_moonfury_dispel);',
                         'RegisterSpellScript(aura_area52_insect_swarm_capstones);',
                         'RegisterSpellScript(aura_area52_natures_splendor);',
                         'RegisterSpellScript(aura_area52_vengeful_soul);',
                         'RegisterSpellScript(aura_area52_natures_reach);'):
        assert registration in capstones, registration
    capstone_sql = (ROOT / 'data/sql/updates/pending_db_world/rev_20261030_20_druid_balance_capstones.sql').read_text(
        encoding='utf-8')
    for row in ('(16913, 72, 0, 0, 0, 0, 65536, 1, 2, 2, 0, 0, 0, 0, 0, 0)',
                '(984419, 0, 7, 0, 0, 524288, 65536, 7, 1, 0, 0, 0, 0, 0, 0, 0)',
                '(816819, 8, 0, 0, 0, 0, 65536, 1, 2, 0, 0, 0, 0, 0, 6000, 0)',
                "(-8921, 'aura_area52_moonfury_dispel')", "(-5570, 'aura_area52_insect_swarm_capstones')",
                "(857866, 'aura_area52_natures_splendor')", "(816913, 'aura_area52_vengeful_soul')",
                "(816819, 'aura_area52_natures_reach')"):
        assert row in capstone_sql, row
    for row in ('UPDATE `spell_proc` SET `SpellFamilyMask2` = 524288 WHERE `SpellId` = -48506;',
                '(16924, 0, 0, 0, 0, 0, 139808, 7, 2, 0, 0, 0, 0, 0, 0, 0)',
                "(16924, 'aura_ascension_celestial_focus')"):
        assert row in sql, row
    with tempfile.TemporaryDirectory(prefix='coa-area52-druid-balance-') as directory:
        out = Path(directory)
        cpp, exe = out / 'balance.cpp', out / 'balance.exe'
        cpp.write_text(HARNESS, encoding='utf-8')
        subprocess.run(command(cpp, exe), cwd=out, check=True, timeout=60)
        subprocess.run([str(exe)], cwd=out, check=True, timeout=15)
    print('area52 druid balance talents: ok')


if __name__ == '__main__':
    main()
