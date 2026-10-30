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
    assert(BeastWithinRageChance(1) == 50 && BeastWithinRageChance(2) == 100);
    assert(FocusedFireApplies(0x10000000, true) && !FocusedFireApplies(0x10000000, false));
    assert(!FocusedFireApplies(0x800, true));
    assert(ModifierTarget(19551, 19615) == false && ModifierTarget(19549, 1583031) == false);
    assert(!ModifierTarget(19551, 13163).has_value() && !ModifierTarget(53265, 19615).has_value());
    assert(FocusedFire == 35029 && KillCommandPetAura == 58914 && TheBeastWithin == 34692);
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
    auras = (ROOT / 'src/server/game/Spells/Auras/SpellAuras.cpp').read_text(encoding='utf-8')
    assert 'if (owner->HasAura(34692))' not in auras
    assert 'owner->GetAuraEffectOfRankedSpell(34692, EFFECT_0)' in auras
    assert 'Area52HunterTalents::BeastWithinRageChance(' in auras
    info = (ROOT / 'src/server/game/Spells/SpellInfo.cpp').read_text(encoding='utf-8')
    assert 'Area52HunterTalents::ModifierTarget(mod->spellId, Id)' in info
    loader = (ROOT / 'src/server/coa/CoAScriptLoader.cpp').read_text(encoding='utf-8')
    assert '    AddSC_Area52HunterBeastMasteryTalents();' in loader
    scripts = (ROOT / 'src/server/coa/Area52HunterBeastMasteryTalents.cpp').read_text(encoding='utf-8')
    assert 'new area52_focused_fire_crit();' in scripts and 'ALLSPELLHOOK_ON_CRIT_CHANCE' in scripts
    sql = (ROOT / 'data/sql/updates/pending_db_world/rev_20261030_50_hunter_beast_mastery_talents.sql').read_text(
        encoding='utf-8')
    for row in ('(-53256, 0, 9, 2048, 524289, 37814272, 69968, 1, 2, 2, 0, 0, 0, 0, 0, 0)',
                '(853253, 0, 9, 0, 0, 8, 81920, 7, 1, 0, 0, 1, 0, 0, 0, 0)'):
        assert row in sql, row
    with tempfile.TemporaryDirectory(prefix='coa-area52-hunter-beast-mastery-') as directory:
        out = Path(directory)
        cpp, exe = out / 'beast_mastery.cpp', out / 'beast_mastery.exe'
        cpp.write_text(HARNESS, encoding='utf-8')
        subprocess.run(command(cpp, exe), cwd=out, check=True, timeout=60)
        subprocess.run([str(exe)], cwd=out, check=True, timeout=15)
    print('area52 hunter beast mastery talents: ok')


if __name__ == '__main__':
    main()
