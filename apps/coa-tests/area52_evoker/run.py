import os
from pathlib import Path
import shutil
import subprocess
import tempfile

HERE = Path(__file__).resolve().parent
ROOT = HERE.parents[2]

HARNESS = r'''#include "Area52EvokerRules.h"
#include <cassert>
using namespace Area52Evoker;

int main()
{
    SpellKey const frostbolt{3, 0x20, 0};
    SpellKey const fireball{3, 0x1, 0};
    SpellKey const missiles{3, 0x800, 0};
    SpellKey const frostfire{3, 0, 0x1000};
    SpellKey const shadowBolt{5, 0x1, 0};
    assert(IsFrostbolt(frostbolt) && !IsFrostbolt(fireball) && IsFireball(fireball) && !IsFireball(shadowBolt));
    assert(IsArcaneMissiles(missiles) && IsFrostfireBolt(frostfire) && !IsFrostfireBolt(fireball));
    assert(ClaimsShatter(fireball, false, false) && !ClaimsShatter(frostbolt, false, false));
    assert(!ClaimsShatter(fireball, true, false) && !ClaimsShatter(fireball, false, true));
    assert(!ClaimsShatter(shadowBolt, false, false));
    ChillStep const first = AddChill(0);
    assert(first.Chills == 1 && !first.Empowers);
    ChillStep const second = AddChill(1);
    assert(second.Chills == 0 && second.Empowers);
    assert(FrostyChanceDurationMs(0) == 1500 && FrostyChanceDurationMs(1200) == 2700);
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
    assert 'HasArea52EvokerHandlers(spell)' in enchant
    build_handlers = enchant.index('HasArea52EvokerHandlers(spell))')
    assert 'Connected.emplace(spell, std::max<std::uint32_t>(1, info->StackAmount));' in enchant[
        build_handlers:build_handlers + 200]
    for registration in ('new area52_evoker_casts();', 'new area52_evoker_damage();', 'new area52_evoked_triggers();'):
        assert registration in enchant, registration
    unit = (ROOT / 'src/server/game/Entities/Unit/Unit.cpp').read_text(encoding='utf-8')
    assert unit.index('GetArea52CritDamageBonusPct(caster, spellProto, victim)') < unit.index(
        'modOwner->ApplySpellMod(spellProto->Id, SPELLMOD_CRIT_DAMAGE_BONUS, crit_bonus);')
    sql = (ROOT / 'data/sql/updates/pending_db_world/rev_20261029_40_evoker_build.sql').read_text(encoding='utf-8')
    for row in ('(783324, 0, 3, 2097152, 0, 0, 65536, 1, 2, 0, 2, 0, 0, 100, 0, 0)',
                "(984624, 0.435, 0, 0.28, 0, 'Area 52 Frosty Frost Frostbolt')",
                "(953308, 'spell_area52_evoker')", "(953309, 'spell_area52_arcane_evoked')",
                "(783324, 'spell_area52_element_bound_missiles')"):
        assert row in sql, row
    with tempfile.TemporaryDirectory(prefix='coa-area52-evoker-') as directory:
        out = Path(directory)
        cpp, exe = out / 'evoker.cpp', out / 'evoker.exe'
        cpp.write_text(HARNESS, encoding='utf-8')
        subprocess.run(command(cpp, exe), cwd=out, check=True, timeout=60)
        subprocess.run([str(exe)], cwd=out, check=True, timeout=15)
    print('area52 evoker rules: ok')


if __name__ == '__main__':
    main()
