import os
from pathlib import Path
import shutil
import subprocess
import tempfile

HERE = Path(__file__).resolve().parent
ROOT = HERE.parents[2]

HARNESS = r'''#include "Area52KnightOfTheEclipseRules.h"
#include <cassert>
using namespace Area52KnightOfTheEclipse;

int main()
{
    assert(NextPhase(Phase::None) == Phase::Solar && NextPhase(Phase::Solar) == Phase::Lunar);
    assert(NextPhase(Phase::Lunar) == Phase::Solar);
    assert(EclipseStrikeDamage(1000, false, 0.0f) == 1000 && EclipseStrikeDamage(1000, true, 0.0f) == 800);
    assert(EclipseStrikeDamage(1000, false, 500.0f) == 1050 && EclipseStrikeDamage(1000, true, 500.0f) == 850);
    assert(SolarBurnChance(1) == 20 && SolarBurnChance(5) == 100 && SolarBurnChance(0) == 0);
    assert(KnightfallDurationMs(5) == 3000 && KnightfallDurationMs(1) == 600);
    assert(WithBonusPct(1000, 6) == 1060 && WithBonusPct(1000, 0) == 1000);
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
    for grant in ('{978226, 978617, 1752, 1752}', '{978226, 978618, 2098, 2098}', '{978226, 979023, 48505, 48505}',
                  '{814183, 814184, 14183, 14183}', '{977831, 977832, 8921}', '{978226, 48525}'):
        assert grant in grants, grant
    contracts = (ROOT / 'src/server/coa/AscensionConditionalCombat.cpp').read_text(encoding='utf-8')
    assert '{954353, 4}, {978621, 7}, {978622, 7}, {979024, 7}, {979045, 8}' in contracts
    sql = (ROOT / 'data/sql/updates/pending_db_world/rev_20261028_50_knight_of_the_eclipse_build.sql').read_text(
        encoding='utf-8')
    for row in ('(978617, 978969, 12)', '(978618, 978981, 12)', '(977832, 977846, 14)',
                '(978621, 0, 7, 1, 0, 524288, 65536, 1, 2, 0, 0, 0, 0, 100, 0, 1)',
                '(978622, 0, 7, 4, 0, 524288, 65536, 1, 2, 0, 0, 0, 0, 100, 0, 1)',
                '(979022, 0, 7, 5, 0, 524288, 65536, 7, 1, 0, 0, 0, 0, 100, 0, 0)',
                '(979046, 0, 0, 0, 0, 0, 65536, 1, 2, 0, 0, 0, 0, 100, 0, 0)',
                '(782152, 0, 7, 2, 0, 0, 262144, 1, 2, 0, 0, 0, 0, 100, 0, 0)',
                "(978623, 0, 0.25522, 0, 0.25522, 'Area 52 Solar Burn')",
                "(-978617, 'spell_area52_eclipse_strike')", "(-978618, 'spell_area52_solar_strike')",
                "(979022, 'spell_area52_knight_haste')", "(48517, 'spell_area52_eclipse_memory')",
                "(48518, 'spell_area52_eclipse_memory')", "(-977832, 'spell_area52_sunfire')",
                "(979046, 'spell_area52_infused_energy')", "(782152, 'spell_area52_demonic_moonfire')",
                "(850286, 'spell_dru_starfall_dummy')", "(850294, 'spell_dru_starfall_aoe')"):
        assert row in sql, row
    with tempfile.TemporaryDirectory(prefix='coa-area52-knight-of-the-eclipse-') as directory:
        out = Path(directory)
        cpp, exe = out / 'knight.cpp', out / 'knight.exe'
        cpp.write_text(HARNESS, encoding='utf-8')
        subprocess.run(command(cpp, exe), cwd=out, check=True, timeout=60)
        subprocess.run([str(exe)], cwd=out, check=True, timeout=15)
    print('area52 knight of the eclipse rules: ok')


if __name__ == '__main__':
    main()
