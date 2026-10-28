import os
from pathlib import Path
import shutil
import subprocess
import tempfile

HERE = Path(__file__).resolve().parent
ROOT = HERE.parents[2]

HARNESS = r'''#include "Area52MarksmanRules.h"
#include "AscensionMysticSpellGrants.h"
#include <cassert>
#include <initializer_list>
using namespace Area52Marksman;
using AscensionMysticEnchant::GrantRankDue;

int main()
{
    ShotEffect steady = CompoundPowerShot(SteadyShot);
    assert(steady.ComboPoints == 2 && !steady.Empowers && steady.Consumes);
    ShotEffect arcane = CompoundPowerShot(ArcaneShot);
    assert(arcane.ComboPoints == 2 && arcane.Empowers && !arcane.Consumes);
    for (unsigned shot : {MultiShot, ChimeraShot, CompoundShot})
    {
        ShotEffect effect = CompoundPowerShot(shot);
        assert(effect.ComboPoints == 0 && effect.Empowers && !effect.Consumes && Triggers(effect));
    }
    assert(!Triggers(CompoundPowerShot(19434)));
    assert(!Triggers(CompoundPowerShot(0)));
    assert(CompoundShot == 826679 && ComboShot == 850515 && CompoundShotRecast == 826680 && RecastWindowMs == 3000);
    assert(GrantRankDue(true, false, 20, 1) && !GrantRankDue(false, true, 80, 60));
    assert(GrantRankDue(false, false, 60, 60) && !GrantRankDue(false, false, 70, 60));
    assert(!GrantRankDue(false, false, 0, 60));
    assert(SyndicateTrainingFor(53302) == 983867 && SyndicateTrainingFor(53304) == SyndicateCapstone);
    assert(SniperTrainingBuffFor(53303) == 64419);
    assert(WithSniperTraining(1000, 28) == 1280 && WithSniperTraining(1000, 0) == 1000);
    assert(HonesIn(4, false) && !HonesIn(3, false) && !HonesIn(4, true));

    unsigned still = 0;
    unsigned stacks = 0;
    for (int second = 1; second <= 9; ++second)
    {
        SniperStep step = SniperTick(still, false);
        still = step.Still;
        assert(!step.Clear);
        stacks += step.AddStack;
        assert(step.AddStack == (second % 3 == 0));
    }
    assert(stacks == 3);
    SniperStep moved = SniperTick(still, true);
    assert(moved.Clear && !moved.AddStack && moved.Still == 0);
    assert(!SniperTick(moved.Still, false).AddStack);
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
    for grant in ('{978716, 978717}', '{978716, 978718}', '{979740, 826679, 19434}', '{979740, 850515, 19434}',
                  '{982890, 982891, 8676}'):
        assert grant in grants, grant
    sql = (ROOT / 'data/sql/updates/pending_db_world/rev_20261028_10_compound_power_build.sql').read_text(
        encoding='utf-8')
    for binding in ("(979740, 'spell_area52_compound_power')", "(-5171, 'spell_area52_compound_power_slice_and_dice')",
                    "(81166, 'spell_area52_sniper')"):
        assert binding in sql, binding
    recast = (ROOT / 'data/sql/updates/pending_db_world/rev_20261028_30_compound_shot_recast.sql').read_text(
        encoding='utf-8')
    for row in ('(826679, 826686, 7)', "(-826679, 'spell_area52_compound_shot')",
                "(826680, 'aura_area52_compound_shot_recast')", "(850517, 'spell_area52_combo_shot_volley')"):
        assert row in recast, row
    assert '{979740, 984925}' in grants
    capstone = (ROOT / 'data/sql/updates/pending_db_world/rev_20261028_31_syndicate_sniper_capstone.sql').read_text(
        encoding='utf-8')
    for row in ('(983869, 0, 9, 131072, 0, 0, 256, 1, 2, 0, 0, 0, 0, 100, 8000, 0)',
                "(-53302, 'spell_area52_syndicate_sniper_training')", "(983869, 'spell_area52_syndicate_capstone')",
                "(982891, 'spell_area52_syndicate_sniper_shot')"):
        assert row in capstone, row
    with tempfile.TemporaryDirectory(prefix='coa-area52-marksman-') as directory:
        out = Path(directory)
        cpp, exe = out / 'marksman.cpp', out / 'marksman.exe'
        cpp.write_text(HARNESS, encoding='utf-8')
        subprocess.run(command(cpp, exe), cwd=out, check=True, timeout=60)
        subprocess.run([str(exe)], cwd=out, check=True, timeout=15)
    print('area52 marksman rules: ok')


if __name__ == '__main__':
    main()
