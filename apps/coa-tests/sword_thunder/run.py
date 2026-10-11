from pathlib import Path
import re
import subprocess
import tempfile

root = Path(__file__).resolve().parents[3]
text = (root / 'src/server/scripts/Spells/spell_warrior.cpp').read_text()
classes = ['spell_warr_sword_and_thunder_modifiers', 'spell_warr_sword_and_thunder_dispel',
           'spell_warr_sword_and_thunder_cleanup', 'spell_warr_sword_and_board']
code = r'''
#include <cassert>
#include <cstdint>
using uint32 = unsigned; using SpellEffIndex = int; using AuraEffectHandleModes = int;
struct ObjectGuid { int value = 0; bool operator==(ObjectGuid const&) const = default; };
struct flag96 { unsigned a,b,c; flag96(unsigned x,unsigned y,unsigned z):a(x),b(y),c(z){} };
constexpr int EFFECT_0=0,EFFECT_1=1,EFFECT_2=2,EFFECT_ALL=3;
constexpr int SPELL_AURA_ADD_PCT_MODIFIER=108,SPELL_AURA_ADD_FLAT_MODIFIER=107;
constexpr int SPELL_AURA_PROC_TRIGGER_SPELL=42,SPELL_EFFECT_SCHOOL_DAMAGE=2,AURA_EFFECT_HANDLE_REAL=1;
constexpr int SPELL_WARRIOR_SWORD_AND_THUNDER=979777,SPELL_WARRIOR_SWORD_AND_BOARD_PROC=50227;
constexpr int SPELL_WARRIOR_SHIELD_SLAM_DISPEL=823922,SPELL_WARRIOR_THUNDER_CLAP=6343;
#define PrepareSpellScript(x) public:
#define PrepareAuraScript(x) public:
#define AuraEffectCalcSpellModFn(...) 0
#define AuraEffectApplyFn(...) 0
#define AuraEffectRemoveFn(...) 0
#define AuraEffectProcFn(...) 0
#define SpellCastFn(...) 0
#define SpellEffectFn(...) 0
struct AuraEffect { int index; int GetEffIndex() const { return index; } };
struct SpellModifier { flag96 mask{0,512,0}; int value=20; };
struct ProcEventInfo {};
struct SpellInfo { int GetCategory() const { return 1280; } };
struct Manager { SpellInfo info; SpellInfo const* GetSpellInfo(int id) { assert(id==6343); return &info; } } manager;
auto* sSpellMgr=&manager;
struct Unit {
 bool enchant=false,proc=false; int casts=0,category=0; ObjectGuid guid{1},selected{2};
 bool HasAura(int id) { return id==979777 ? enchant : id==50227 && proc; }
 bool IsPlayer() { return true; } Unit* ToPlayer() { return this; }
 ObjectGuid GetGUID() { return guid; } ObjectGuid GetTarget() { return selected; }
 void CastSpell(Unit* target,int id,bool triggered) { assert(target && id==823922 && triggered); ++casts; }
 void RemoveAurasDueToSpell(int id) { assert(id==50227);proc=false; }
 void RemoveCategoryCooldown(int id) { category=id; }
};
using Player=Unit;
struct Script {
 Unit caster,target; int DoEffectCalcSpellMod=0,BeforeCast=0,OnEffectHitTarget=0;
 int AfterEffectApply=0,AfterEffectRemove=0,OnEffectProc=0;
 virtual void Register() {} Unit* GetCaster(){return &caster;} Unit* GetHitUnit(){return &target;}
 Unit* GetTarget(){return &caster;} Unit* GetUnitOwner(){return &caster;}
};
using SpellScript=Script; using AuraScript=Script;
'''
for name in classes:
    match = re.search(r'class ' + name + r' : public (?:SpellScript|AuraScript)\n\{.*?\n\};', text, re.S)
    assert match, name
    code += match.group(0) + '\n'
code += r'''
int main() {
 spell_warr_sword_and_thunder_modifiers mods;
 AuraEffect effect{0}; SpellModifier value; SpellModifier* pointer=&value;
 mods.CalculateModifier(&effect,pointer); assert(value.mask.b==512 && value.value==20);
 mods.caster.enchant=true;
 mods.CalculateModifier(&effect,pointer); assert(value.mask.a==128 && value.mask.b==0 && value.value==-100);
 effect.index=1;value.value=20;mods.CalculateModifier(&effect,pointer);assert(value.mask.a==128 && value.value==20);
 effect.index=2;mods.CalculateModifier(&effect,pointer);assert(value.mask.a==0 && value.mask.b==0 && value.value==0);
 spell_warr_sword_and_thunder_dispel spell;
 spell.caster.enchant=true;spell.caster.proc=true;spell.CaptureProc();
 spell.target.guid={3};spell.DispelTarget(0);assert(spell.caster.casts==0);
 spell.target.guid={2};spell.DispelTarget(0);assert(spell.caster.casts==1);
 spell.caster.proc=false;spell.CaptureProc();spell.DispelTarget(0);assert(spell.caster.casts==1);
 spell.caster.proc=true;spell.caster.enchant=false;spell.CaptureProc();spell.DispelTarget(0);assert(spell.caster.casts==1);
 spell.caster.enchant=true;spell.caster.selected={0};spell.CaptureProc();spell.DispelTarget(0);assert(spell.caster.casts==1);
 spell_warr_sword_and_thunder_cleanup cleanup;cleanup.caster.proc=true;cleanup.ClearProc(nullptr,1);assert(!cleanup.caster.proc);
 spell_warr_sword_and_board talent;ProcEventInfo event;talent.HandleProc(nullptr,event);assert(talent.caster.category==1209);
 talent.caster.enchant=true;talent.HandleProc(nullptr,event);assert(talent.caster.category==1280);
}
'''
with tempfile.TemporaryDirectory() as directory:
    source = Path(directory) / 'test.cpp'
    binary = Path(directory) / 'test'
    source.write_text(code)
    subprocess.run(['c++', '-std=c++20', str(source), '-o', str(binary)], check=True)
    subprocess.run([str(binary)], check=True)
print('PASS: scoped cost/threat, cooldown redirection, selected target only, no proc, no enchant, no selection, cleanup')
