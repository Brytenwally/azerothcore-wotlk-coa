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
    assert(PctOf(1000, RevitalizeRefundPct) == 300 && PctOf(5000, 30) == 1500 && PctOf(0, 50) == 0);
    assert(LivingSpiritApplies(49.9f) && !LivingSpiritApplies(50.0f) && !LivingSpiritApplies(80.0f));
    assert(LivingSpiritBonusPct == 50 && CapstoneThreatReduction == 957339 && NaturesFocusHeal == 717067);
    assert(Contains(IntensityMana, 835359) && !Contains(IntensityMana, 35359));
    assert(Contains(TreeForms, 33891) && Contains(TreeForms, 86435));
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
    corrections = (ROOT / 'src/server/game/Spells/SpellInfoCorrections.cpp').read_text(encoding='utf-8')
    for spell, mask in (('17078', 'flag96(0x40, 0, 0)'), ('51183', 'flag96(0x10, 0, 0)')):
        fix = corrections.index('ApplySpellFix({ ' + spell + ' }')
        block = corrections[fix:fix + 250]
        assert 'ApplyAuraName = SPELL_AURA_PERIODIC_HASTE;' in block and mask in block, spell
    auras = (ROOT / 'src/server/game/Spells/Auras/SpellAuraEffects.cpp').read_text(encoding='utf-8')
    assert 'Stances & (1 << (GetMiscValue() - 1))' not in auras
    assert auras.count('Stances & (GetMiscValue() <= 32 ? (1u << (GetMiscValue() - 1)) : 0u)') == 4
    loader = (ROOT / 'src/server/coa/CoAScriptLoader.cpp').read_text(encoding='utf-8')
    assert '    AddSC_Area52DruidRestorationTalents();' in loader
    scripts = (ROOT / 'src/server/coa/Area52DruidRestorationTalents.cpp').read_text(encoding='utf-8')
    for registration in ('RegisterSpellScript(aura_area52_capstone_grant);',
                         'RegisterSpellScript(aura_area52_natures_focus_guard);',
                         'RegisterSpellScript(spell_area52_intensity_mana);',
                         'RegisterSpellScript(aura_area52_improved_tree_of_life);',
                         'RegisterSpellScript(aura_area52_tree_form_talents);',
                         'new area52_druid_restoration_capstones();', 'new area52_living_spirit();'):
        assert registration in scripts, registration
    sql = (ROOT / 'data/sql/updates/pending_db_world/rev_20261030_40_druid_restoration_talents.sql').read_text(
        encoding='utf-8')
    for row in ('(816860, 0, 0, 0, 0, 0, 81924, 3, 2, 0, 0, 0, 0, 0, 2000, 0)',
                '(816861, 0, 0, 0, 0, 0, 81924, 3, 2, 0, 0, 0, 0, 0, 2000, 0)',
                '(33880, 0, 7, 32, 0, 131072, 0, 7, 1, 0, 0, 0, 0, 0, 0, 0)',
                '(717066, 0, 0, 0, 0, 0, 139944, 1, 2, 2, 0, 0, 0, 0, 0, 0)',
                "(816861, 'spell_dru_omen_of_clarity')", "(17066, 'aura_area52_capstone_grant')",
                "(17120, 'aura_area52_capstone_grant')", "(717066, 'aura_area52_natures_focus_guard')",
                "(835359, 'spell_area52_intensity_mana')", "(-48535, 'aura_area52_improved_tree_of_life')",
                "(86435, 'aura_area52_tree_form_talents')"):
        assert row in sql, row
    with tempfile.TemporaryDirectory(prefix='coa-area52-druid-restoration-') as directory:
        out = Path(directory)
        cpp, exe = out / 'restoration.cpp', out / 'restoration.exe'
        cpp.write_text(HARNESS, encoding='utf-8')
        subprocess.run(command(cpp, exe), cwd=out, check=True, timeout=60)
        subprocess.run([str(exe)], cwd=out, check=True, timeout=15)
    print('area52 druid restoration talents: ok')


if __name__ == '__main__':
    main()
