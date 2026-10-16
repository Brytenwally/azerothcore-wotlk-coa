import itertools
from pathlib import Path
import shutil
import subprocess
import tempfile

ROOT = Path(__file__).resolve().parents[3]

def main():
    compiler = shutil.which('g++') or shutil.which('clang++')
    assert compiler, 'C++ compiler unavailable'
    spells = [11232, 12500, 12501, 31584, 31585, 31586, 11222, 12839, 12840,
              28574, 54658, 54659, 31682, 31683, 31684, 831682, 701464,
              828574, 828575, 828576]
    targets = {31682: {831682}, 31683: {831682}, 31684: {831682},
               54659: {701464}, 701464: {28574, 54658, 54659}}
    assertions = []
    for modifier, allowed in targets.items():
        for target in spells:
            expected = 'true' if target in allowed else 'false'
            assertions.append(f'assert(AscensionMageTalentTarget({modifier}, {target}) == {expected});')
    for order in itertools.permutations(targets):
        for target in (11232, 12501, 31584, 11222):
            assertions.append('value = 4;')
            for modifier in order:
                assertions.append(f'if (AscensionMageTalentTarget({modifier}, {target}).value_or(false)) value += 100;')
            assertions.append('assert(value == 4);')
    for modifier in (828574, 828575, 828576):
        for target in spells:
            assertions.append(f'assert(AscensionMageTalentTarget({modifier}, {target}) == false);')
    for unknown in (12345,):
        assertions.append(f'assert(!AscensionMageTalentTarget({unknown}, 11232).has_value());')
    assertions += [
        'using namespace AscensionPowerSurge;',
        'assert(Chance(0) == 0 && Chance(1) == 2 && Chance(2) == 4 && Chance(3) == 6);',
        'assert(Burst(2)==828597 && Burst(4)==828591 && Burst(8)==828594);',
        'assert(Burst(16)==828595 && Burst(32)==828596 && Burst(64)==828590);',
        'ReturnState state; state.Capture(777, 333); state.Finish(true);',
        'unsigned total = 0; for(unsigned i=0;i<10;++i) total += state.Tick();',
        'assert(total==1110 && state.remaining==0 && state.ticks==0 && state.Tick()==0);',
        'state.Capture(5, 2); state.Finish(true); total=0;',
        'for(unsigned i=0;i<10;++i) { total+=state.Tick(); } assert(total==7);',
        'state.Capture(999, 888); state.Finish(false); assert(state.stored==0 && state.Tick()==0);',
    ]
    source = '#include <cassert>\n#include "AscensionMageTalentTargets.h"\n#include "AscensionPowerSurgeRules.h"\nint main(){int value;\n'
    source += '\n'.join(assertions) + '\n}\n'
    with tempfile.TemporaryDirectory() as temporary:
        directory = Path(temporary)
        (directory / 'test.cpp').write_text(source)
        binary = directory / 'test'
        subprocess.run([compiler, '-std=c++17', '-Wall', '-Wextra', '-Werror',
                        '-I', str(ROOT / 'src/server/coa'), str(directory / 'test.cpp'), '-o', str(binary)], check=True)
        subprocess.run([str(binary)], check=True)
        unit_text = (ROOT / 'src/server/game/Entities/Unit/Unit.cpp').read_text()
        start = unit_text.index('float Unit::ApplyEffectModifiers(')
        end = unit_text.index('\n}\n', start) + 3
        native = unit_text[start:end]
        effect_test = r'''
#include <algorithm>
#include <cassert>
#include <vector>
#include "AscensionMageTalentTargets.h"
using uint8 = unsigned char;
constexpr int SPELLMOD_ALL_EFFECTS=8, SPELLMOD_EFFECT1=3, SPELLMOD_EFFECT2=12, SPELLMOD_EFFECT3=23;
struct SpellInfo { unsigned Id; };
struct Mod { unsigned id; int op; float amount; bool percent; };
struct Player {
    std::vector<Mod> mods;
    void ApplySpellMod(unsigned id, int op, float& value) {
        for(auto const& mod: mods)
            if(mod.op == op && AscensionMageTalentTarget(mod.id,id).value_or(false))
                value = mod.percent ? value*(1+mod.amount/100) : value+mod.amount;
    }
};
struct Unit {
    Player* player;
    Player* GetSpellModOwner() const { return player; }
    float ApplyEffectModifiers(SpellInfo const*, uint8, float) const;
};
NATIVE_FUNCTION
int main() {
    std::vector<Mod> choices={{31682,3,7,false},{31682,12,3,false},
        {54659,3,30,false},{701464,3,100,true},{828574,3,2,false}};
    std::vector<int> order={0,1,2,3,4};
    do {
        Player player;
        Unit unit{&player};
        for(int index:order) {
            player.mods.push_back(choices[index]);
            for(unsigned id:{11232u,12500u,12501u,31584u,31585u,31586u,11222u,12839u,12840u}) {
                SpellInfo spell{id};
                for(float base:{2.f,3.f,4.f,6.f,7.f,9.f,10.f})
                    assert(unit.ApplyEffectModifiers(&spell,0,base)==base);
            }
        }
        for(unsigned id:{28574u,54658u,54659u}) {
            SpellInfo spell{id};
            for(float base:{50.f,100.f,150.f})
                assert(unit.ApplyEffectModifiers(&spell,0,base)==base*2);
        }
        while(!player.mods.empty()) {
            player.mods.pop_back();
            SpellInfo mind{11232};
            assert(unit.ApplyEffectModifiers(&mind,0,4)==4);
        }
    } while(std::next_permutation(order.begin(),order.end()));
}
'''
        (directory / 'effect.cpp').write_text(effect_test.replace('NATIVE_FUNCTION', native))
        subprocess.run([compiler, '-std=c++17', '-Wall', '-Wextra', '-Werror', '-I', str(ROOT / 'src/server/coa'),
                        str(directory / 'effect.cpp'), '-o', str(binary)], check=True)
        subprocess.run([str(binary)], check=True)
        implementation = (ROOT / 'src/server/coa/AscensionPowerSurge.inc').read_text()
        implementation = '\n'.join(line for line in implementation.splitlines() if not line.startswith('#include'))
        mock = (Path(__file__).parent / 'power_surge_mock.cpp').read_text()
        (directory / 'mock.cpp').write_text(mock.replace('PRODUCTION_IMPLEMENTATION', implementation))
        subprocess.run([compiler, '-std=c++17', '-Wall', '-Wextra', '-Werror',
                        '-I', str(ROOT / 'src/server/coa'), str(directory / 'mock.cpp'), '-o', str(binary)], check=True)
        subprocess.run([str(binary)], check=True)
    text = (ROOT / 'src/server/game/Spells/SpellInfo.cpp').read_text()
    assert 'AscensionMageTalentTarget(mod->spellId, Id)' in text
    print('PASS: 100 production target decisions; 120 modifier orders preserve unrelated amounts; unknown modifiers retain fallback. Scope: targeting only, not aura lifecycle or gameplay.')
    print('PASS: production Power Surge handlers with mocks: ranks, six schools, eligibility, absorption, completed/interrupted channel, ten-second return, rounding, death and rank removal.')
    print('PASS: extracted core ApplyEffectModifiers preserves talent amounts across 120 add/remove orders and doubles all Fortitude rank amounts with Barrier. Player modifier storage is mocked.')

if __name__ == '__main__':
    main()
