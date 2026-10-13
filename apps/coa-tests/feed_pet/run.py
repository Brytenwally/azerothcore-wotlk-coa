from pathlib import Path
import subprocess
import tempfile
root = Path(__file__).resolve().parents[3]
text = (root / "src/server/game/Spells/SpellEffects.cpp").read_text()
body = text.split("void Spell::EffectFeedPet(SpellEffIndex effIndex)", 1)[1].split("void Spell::EffectDismissPet", 1)[0]
stubs = r"""
#include <cassert>
#include <cstdint>
using uint32 = uint32_t;
using int32 = int32_t;
using SpellEffIndex = int;
constexpr int SPELL_EFFECT_HANDLE_HIT_TARGET=1, EFFECT_0=0, SPELL_AURA_PERIODIC_ENERGIZE=24, POWER_HAPPINESS=4;
struct Effect { uint32 TriggerSpell=1539; int ApplyAuraName=0; int MiscValue=0; };
struct SpellInfo { Effect Effects[3]; };
struct Manager { SpellInfo info; SpellInfo const* GetSpellInfo(uint32) {return &info;} } manager;
auto sSpellMgr=&manager;
struct Item { struct Template {int ItemLevel=1;} value; Template* GetTemplate(){return &value;} uint32 GetEntry(){return 1;} };
struct Pet { bool alive=true; int benefit=35000; bool IsAlive(){return alive;} int GetCurrentFoodBenefitLevel(int){return benefit;} };
struct Player { Pet pet; int used=0, amount=0, calls=0; Player* ToPlayer(){return this;} Pet* GetPet(){return &pet;}
void DestroyItemCount(Item*,uint32 count,bool){used+=count;}
void CastCustomSpell(Pet*,uint32,int* n,void*,void*,bool){amount=*n;calls++;}
void CastSpell(Pet*,uint32,bool){amount=50;calls++;} };
struct Spell { int effectHandleMode=1; Player* m_caster; Item* itemTarget; SpellInfo* m_spellInfo;
void ExecuteLogEffectDestroyItem(int,uint32){} void EffectFeedPet(SpellEffIndex); };
"""
tests = r"""
int main() {
for (int food : {35000,17000,8000}) {
 Player player; player.pet.benefit=food; Item item; SpellInfo info; Spell spell{1,&player,&item,&info};
 manager.info.Effects[0].ApplyAuraName=0;
 spell.EffectFeedPet(0); assert(player.amount==50); assert(player.used==1); assert(player.calls==1);
 assert(1000*player.amount/100==500);
 manager.info.Effects[0].ApplyAuraName=24; manager.info.Effects[0].MiscValue=4;
 spell.EffectFeedPet(0); assert(player.amount==food); assert(player.used==2);
}
Player player; Item item; SpellInfo info; Spell spell{1,&player,&item,&info};
player.pet.benefit=0;spell.EffectFeedPet(0);assert(player.used==0 && player.calls==0);
player.pet.benefit=35000;player.pet.alive=false;spell.EffectFeedPet(0);assert(player.used==0 && player.calls==0);
}
"""
with tempfile.TemporaryDirectory() as directory:
 p = Path(directory)
 (p / "test.cpp").write_text("#include <initializer_list>\n" + stubs + "void Spell::EffectFeedPet(SpellEffIndex effIndex)" + body + tests)
 subprocess.run(["c++", "-std=c++20", str(p / "test.cpp"), "-o", str(p / "test")], check=True)
 subprocess.run([str(p / "test")], check=True)
print("Actual feeding handler preserves healing amount and happiness food tiers; invalid feeding consumes nothing")
