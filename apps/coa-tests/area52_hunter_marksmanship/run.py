import os
from pathlib import Path
import shutil
import subprocess
import tempfile

HERE = Path(__file__).resolve().parent
ROOT = HERE.parents[2]

HARNESS = r'''#include "Area52HunterTalentRules.h"
#include <cassert>
using namespace Area52HunterTalents;

int main()
{
    assert(RankGrantHelper(34482) == 834482 && RankGrantHelper(34484) == 834484);
    assert(RankGrantHelper(34948) == 984492 && RankGrantHelper(834949) == 984494);
    assert(RankGrantHelper(53234) == 984498 && RankGrantHelper(53238) == 984500);
    assert(RankGrantHelper(53221) == 0);
    assert(MortalShotsAimedShotDurationMs(19485) == 1000 && MortalShotsAimedShotDurationMs(19488) == 3000);
    assert(MortalShotsAimedShotDurationMs(19489) == 0);
    assert(SteadyShotCapstoneApplies(true, false) && !SteadyShotCapstoneApplies(true, true));
    assert(!SteadyShotCapstoneApplies(false, false));
    assert(ImprovedSteadyShotCapstone == 53224 && CapstoneSteadyShotBuff == 853220 && CapstoneArmorPierce == 853221);
    assert(SerpentsKiss == 851978 && SerpentsKissDamage == 851979 && ImprovedStingsCapstone == 19466);
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
    assert 'flag96(399361, 276955137, 1)' not in corrections
    for spell, line in (('19466', 'Effects[EFFECT_1].MiscValue = SPELLMOD_ALL_EFFECTS;'),
                        ('35111', 'Effects[EFFECT_2].TargetA = SpellImplicitTargetInfo(TARGET_UNIT_CASTER);'),
                        ('435104, 435105', 'Effects[EFFECT_0].Amplitude = 1000;')):
        fix = corrections.index('ApplySpellFix({ ' + spell + ' }')
        assert line in corrections[fix:fix + 200], spell
    unit = (ROOT / 'src/server/game/Entities/Unit/Unit.cpp').read_text(encoding='utf-8')
    assert unit.count('case 417801:') == 2 and unit.count('case 417803:') == 2
    loader = (ROOT / 'src/server/coa/CoAScriptLoader.cpp').read_text(encoding='utf-8')
    assert '    AddSC_Area52HunterMarksmanshipTalents();' in loader
    scripts = (ROOT / 'src/server/coa/Area52HunterMarksmanshipTalents.cpp').read_text(encoding='utf-8')
    for registration in ('RegisterSpellScript(aura_area52_hunter_rank_grant);',
                         'RegisterSpellScript(aura_area52_mortal_shots);',
                         'RegisterSpellScript(aura_area52_improved_trueshot_aura);',
                         'RegisterSpellScript(aura_area52_improved_steady_shot);',
                         'RegisterSpellScript(aura_area52_capstone_steady_shot_buff);',
                         'RegisterSpellScript(aura_area52_serpent_sting_dispel);',
                         'new area52_hunter_marksmanship_capstones();'):
        assert registration in scripts, registration
    sql = (ROOT / 'data/sql/updates/pending_db_world/rev_20261030_60_hunter_marksmanship_talents.sql').read_text(
        encoding='utf-8')
    for row in ('(35098, 0, 0, 0, 0, 0, 0, 1, 2, 0, 8, 7, 0, 0, 0, 2)',
                '(835099, 0, 0, 0, 0, 0, 0, 1, 2, 0, 8, 7, 0, 0, 0, 2)',
                '(-53215, 0, 9, 1, 1, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0)',
                '(-53234, 0, 9, 137216, 8388609, 1, 0, 0, 2, 2, 0, 0, 0, 0, 0, 0)',
                "(834949, 'aura_area52_hunter_rank_grant')", "(53238, 'aura_area52_hunter_rank_grant')",
                "(19488, 'aura_area52_mortal_shots')", "(435105, 'aura_area52_improved_trueshot_aura')",
                "(-53221, 'aura_area52_improved_steady_shot')",
                "(853220, 'aura_area52_capstone_steady_shot_buff')", "(-1978, 'aura_area52_serpent_sting_dispel')"):
        assert row in sql, row
    with tempfile.TemporaryDirectory(prefix='coa-area52-hunter-marksmanship-') as directory:
        out = Path(directory)
        cpp, exe = out / 'marksmanship.cpp', out / 'marksmanship.exe'
        cpp.write_text(HARNESS, encoding='utf-8')
        subprocess.run(command(cpp, exe), cwd=out, check=True, timeout=60)
        subprocess.run([str(exe)], cwd=out, check=True, timeout=15)
    print('area52 hunter marksmanship talents: ok')


if __name__ == '__main__':
    main()
