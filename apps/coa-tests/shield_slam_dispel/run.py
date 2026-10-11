from pathlib import Path
import re
import subprocess
import tempfile

root = Path(__file__).resolve().parents[3]
text = (root / 'src/server/scripts/Spells/spell_warrior.cpp').read_text()
match = re.search(r'class spell_warr_shield_slam_dispel_gate : public SpellScript\n\{.*?\n\};', text, re.S)

assert match, 'Shield Slam dispel gate is missing'
code = r'''
#include <cassert>
using SpellEffIndex = int;
constexpr int EFFECT_0 = 0, SPELL_EFFECT_TRIGGER_SPELL = 64;
constexpr int SPELL_WARRIOR_SWORD_AND_BOARD_PROC = 50227;
constexpr int SPELL_WARRIOR_SHIELD_SLAM_DISPEL = 823922;
constexpr int SPELL_WARRIOR_SWORD_AND_THUNDER = 979777;
#define PrepareSpellScript(x) public:
#define SpellEffectFn(a,b,c) 0
struct Effect { int TriggerSpell = 823922; };
struct Info { Effect Effects[3]; };
struct Caster {
    bool proc = false;
    bool enchant = false;
    bool HasAura(int id) { return id == 979777 ? enchant : id == 50227 && proc; }
};
struct SpellScript {
    Info info;
    Caster caster;
    bool prevented = false;
    int OnEffectLaunchTarget = 0;
    virtual void Register() {}
    Info* GetSpellInfo() { return &info; }
    Caster* GetCaster() { return &caster; }
    void PreventHitDefaultEffect(int index) { assert(index == 0); prevented = true; }
};
'''
code += match.group(0)
code += r'''
int main() {
    for (bool proc : {false, true}) {
        spell_warr_shield_slam_dispel_gate spell;
        spell.caster.proc = proc;
        spell.HandleDispelTrigger(0);
        assert(spell.prevented == !proc);
    }
    spell_warr_shield_slam_dispel_gate redirected;
    redirected.caster.proc = true;
    redirected.caster.enchant = true;
    redirected.HandleDispelTrigger(0);
    assert(redirected.prevented);
    spell_warr_shield_slam_dispel_gate other;
    other.info.Effects[0].TriggerSpell = 1;
    other.HandleDispelTrigger(0);
    assert(!other.prevented);
}
'''
code = '#include <initializer_list>\n' + code
sql = (root / 'data/sql/updates/pending_db_world/rev_20261011_03_shield_slam_dispel.sql').read_text()
assert "(-23922, 'spell_warr_shield_slam_dispel_gate')" in sql
assert 'RegisterSpellScript(spell_warr_shield_slam_dispel_gate);' in text
with tempfile.TemporaryDirectory() as directory:
    source = Path(directory) / 'gate.cpp'
    binary = Path(directory) / 'gate'
    source.write_text(code)
    subprocess.run(['c++', '-std=c++20', str(source), '-o', str(binary)], check=True)
    subprocess.run([str(binary)], check=True)
print('PASS: proc absent, proc present, unrelated trigger, all-rank binding')
