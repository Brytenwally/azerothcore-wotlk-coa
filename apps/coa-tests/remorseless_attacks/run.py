from pathlib import Path
import shutil
import subprocess
import tempfile

root = Path(__file__).resolve().parents[3]
source = (root / 'src/server/scripts/Spells/spell_rogue.cpp').read_text()
body = source.split('class spell_rog_remorseless_attacks : public AuraScript', 1)[1].split('// -51664', 1)[0]
body = 'class spell_rog_remorseless_attacks : public AuraScript' + body
body = body.replace('    PrepareAuraScript(spell_rog_remorseless_attacks);', 'public:')
body = body[:body.index('    void Register() override')] + '};'
stub = r"""
#include <cassert>
struct SpellInfo { int GetMaxDuration() const { return 21000; } };
struct Aura { int duration=1000, maximum=1000; int bonus=0;
void SetDuration(int n,bool mods) { duration=n+(mods?bonus:0); }
void SetMaxDuration(int n) { maximum=n; } int GetDuration() { return duration; } };
struct AuraEffect { mutable Aura aura; SpellInfo info;
Aura* GetBase() const { return &aura; } SpellInfo const* GetSpellInfo() const { return &info; } };
struct Unit { AuraEffect* snd=nullptr; int GetGUID() { return 9; }
AuraEffect const* GetAuraEffect(int,int,int mask,int,int,int guid) {
assert(mask==0x40000 && guid==9); return snd; } };
struct ProcEventInfo {};
struct AuraScript { Unit owner; int id=14148; int GetId() { return id; } Unit* GetTarget() { return &owner; } };
constexpr int SPELL_AURA_MOD_MELEE_HASTE=1, SPELLFAMILY_ROGUE=8;
"""
main = r"""
int main() {
spell_rog_remorseless_attacks s; ProcEventInfo e;
s.AfterKill(nullptr,e);
AuraEffect effect; s.owner.snd=&effect; s.id=14144; s.AfterKill(nullptr,e);
assert(effect.aura.duration==1000);
s.id=14148; s.AfterKill(nullptr,e); assert(effect.aura.duration==21000 && effect.aura.maximum==21000);
effect.aura.bonus=10500; s.AfterKill(nullptr,e); assert(effect.aura.duration==31500);
effect.aura.duration=500; s.AfterKill(nullptr,e); assert(effect.aura.duration==31500);
}
"""
assert 'AfterEffectProc += AuraEffectProcFn(spell_rog_remorseless_attacks::AfterKill,' in source
assert 'EFFECT_0, SPELL_AURA_PROC_TRIGGER_SPELL' in source
sql = (root / 'data/sql/updates/pending_db_world/rev_20261013_01_remorseless_attacks.sql').read_text()
assert "VALUES (14148, 'spell_rog_remorseless_attacks')" in sql
with tempfile.TemporaryDirectory() as directory:
    folder = Path(directory)
    (folder / 'test.cpp').write_text(stub + body + main)
    subprocess.run([shutil.which('g++'), '-std=c++20', str(folder / 'test.cpp'), '-o', str(folder / 'test')], check=True)
    subprocess.run([str(folder / 'test')], check=True)
print('Capstone: absent aura, rank gate, maximum duration, modifiers and repeated refresh passed')
