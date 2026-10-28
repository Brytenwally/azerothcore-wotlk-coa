from pathlib import Path
import subprocess,tempfile
R=Path(__file__).resolve().parents[3]/'src/server/coa'
def body(text, signature):
 start=text.index('{',text.index(signature)); depth=1;end=start+1
 while depth:
  depth+=(text[end]=='{')-(text[end]=='}');end+=1
 return text[start:end]
dragon=(R/'Area52DragonWarrior.inc').read_text()
marksman=(R/'Area52Marksman.inc').read_text()
cpp=r'''
#include "Area52MarksmanRules.h"
#include "Area52DragonWarriorRules.h"
#include <cassert>
#include <map>
#include <string>
#include <cmath>
#include <algorithm>
#include <set>
#include <vector>
using uint32=unsigned; using int32=int; using SpellEffIndex=int;
constexpr int EFFECT_0=0;
constexpr int EFFECT_1=1,EFFECT_2=2,SPELL_EFFECT_APPLY_AURA=6,TARGET_UNIT_CASTER=1;
constexpr int SPELL_AURA_ADD_PCT_MODIFIER=108,SPELL_ATTR0_CU_IGNORE_ARMOR=1,SPELL_EFFECT_NONE=0;
constexpr int SPELL_AURA_MOD_STEALTH=16,RANGED_ATTACK=2;
constexpr int SPELL_AURA_MOD_RANGED_HASTE=140,AURA_EFFECT_HANDLE_REAL=1;
struct SpellImplicitTargetInfo { int value; SpellImplicitTargetInfo(int v=0):value(v){} };
struct SpellInfo { unsigned Id; unsigned AttributesCu=0; struct EffectData { int Effect=0,ApplyAuraName=0,BasePoints=0; SpellImplicitTargetInfo TargetA; int CalcValue(void*)const{return BasePoints+1;} } Effects[3]; };
struct SpellMgr { SpellInfo bonus{}; std::map<unsigned,unsigned> roots,next;
 unsigned GetFirstSpellInChain(unsigned id){return roots.count(id)?roots[id]:id;}
 unsigned GetNextSpellInChain(unsigned id){return next[id];}
 SpellInfo const* GetSpellInfo(unsigned){return &bonus;} } manager;
SpellMgr* sSpellMgr=&manager;
struct Config {
 bool enabled=true; std::string model="coa",realm="live";unsigned mask=0;
 template<class T>T GetOption(char const* key,T){
  if constexpr(std::is_same_v<T,bool>)return enabled;
  else if constexpr(std::is_same_v<T,std::string>)return std::string(key)=="CoA.ClassModel"?model:realm;
  else return mask;
 }
} config;
Config* sConfigMgr=&config;
struct Metadata { void Apply(SpellInfo* info) BODY_METADATA };
struct AuraEffect { int amount; unsigned id=0,recalculated=0; int GetAmount()const{return amount;}
 unsigned GetId()const{return id;} void RecalculateAmount(){++recalculated;} };
struct Player {
 std::set<unsigned> known;std::map<unsigned,unsigned> m_temporarySpellReplacements;
 std::vector<AuraEffect*> rangedEffects;
 auto const& GetAuraEffectsByType(int)const{return rangedEffects;}
 bool HasActiveSpell(unsigned id)const{return known.count(id);}
 std::vector<uint32> GetSpellInteractionOrigins(uint32 spellId) const;
 bool area=true,stealth=false; unsigned now=0; std::map<unsigned,unsigned> cooldown;
 std::map<unsigned,AuraEffect> auras;
 Player* ToPlayer(){return this;}
 bool HasSpellCooldown(unsigned id){return cooldown[id]>now;}
 void AddSpellCooldown(unsigned id,unsigned,unsigned duration){cooldown[id]=now+duration;}
 AuraEffect const* GetAuraEffect(unsigned id,int){auto it=auras.find(id);return it==auras.end()?nullptr:&it->second;}
 void RemoveAurasDueToSpell(unsigned id){auras.erase(id);}
 bool HasAura(unsigned id)const{return auras.count(id);}
 bool HasAuraType(int){return stealth;}
 bool MatchesSpellInteraction(unsigned spell,unsigned original){return spell==original || (spell==999999 && original==56641);}
};
using Unit=Player;
namespace AscensionFreePick { bool Applies(Player const* p){return p && p->area;} }
struct Skull {
 Player* caster; bool prevented=false;
 Unit* GetCaster(){return caster;}
 void PreventHitDefaultEffect(int){prevented=true;}
 void LimitCooldown(SpellEffIndex index) BODY_SKULL
};
struct Shot {
 Player* caster; int damage=1000; int HoneInBonus=0;int StealthBonus=0;
 Unit* GetCaster(){return caster;}
 int GetHitDamage(){return damage;}
 void SetHitDamage(int v){damage=v;}
 void CaptureHoneIn() BODY_CAPTURE
 void SpendHoneIn() BODY_SPEND
 void AddSniperTraining() BODY_DAMAGE
 bool Load() BODY_SHOT_LOAD
};
struct CastUnit: Player {
 float m_modAttackSpeedPct[3]={1,1,1};
 void CastTime(SpellInfo const* spellInfo,int32& castTime){switch(1){case 1: BODY_RANGED default:break;}}
};
using AuraEffectHandleModes=int;
struct Cleanup {
 Player* target; unsigned id;
 Player* GetTarget(){return target;}
 unsigned GetId(){return id;}
 void Clear(AuraEffect const*, AuraEffectHandleModes) BODY_CLEANUP
 void RefreshSliceAndDice(AuraEffect const*, AuraEffectHandleModes) BODY_REFRESH
};
int main(){
 Player inherited;inherited.known={826679,826681,850515,19434,20900};
 manager.roots[826681]=826679;manager.next[19434]=20900;
 assert(inherited.GetSpellInteractionOrigins(826681).empty());
 inherited.auras[979740]={1};
 assert(inherited.GetSpellInteractionOrigins(826681)==std::vector<uint32>({19434,20900}));
 inherited.m_temporarySpellReplacements[826681]=850515;
 auto origins=inherited.GetSpellInteractionOrigins(850515);
 assert(origins==std::vector<uint32>({826681,19434,20900}));
 inherited.auras.erase(979740);assert(inherited.GetSpellInteractionOrigins(826681).empty());
 inherited.auras[979740]={1};inherited.known.erase(19434);inherited.known.erase(20900);
 assert(inherited.GetSpellInteractionOrigins(826681).empty());
 inherited.area=false;assert(inherited.GetSpellInteractionOrigins(850515).empty());
 Metadata metadata; SpellInfo cap{};cap.Id=983869;metadata.Apply(&cap);assert(cap.Effects[2].Effect==0);
 config.model="hero";metadata.Apply(&cap);assert(cap.Effects[2].Effect==6 && cap.Effects[2].TargetA.value==1);
 metadata.Apply(&cap);assert(cap.Effects[2].Effect==6);
 SpellInfo slam{};slam.Id=954261;metadata.Apply(&slam);assert(slam.Effects[1].Effect==6);
 SpellInfo volley{};volley.Id=850517;metadata.Apply(&volley);assert(volley.Effects[1].Effect==6);
 SpellInfo roar{};roar.Id=954250;metadata.Apply(&roar);assert(roar.AttributesCu & SPELL_ATTR0_CU_IGNORE_ARMOR);
 for(unsigned rank=0;rank<3;++rank){
  SpellInfo training{};training.Id=983867+rank;metadata.Apply(&training);
  assert(training.Effects[1].ApplyAuraName==108);
  assert(training.Effects[1].BasePoints+1==-int(1+2*rank));
  for(int stacks=1;stacks<=4;++stacks){
   float pct=1+(training.Effects[1].BasePoints+1)*stacks/100.0f;
   assert(std::fabs(2500*pct-(2500-25*(1+2*rank)*stacks))<0.01f);
   for(float base:{1000.0f,2500.0f,4000.0f})for(float haste:{0.8f,1.0f,1.3f}){
    float expected=base*(100-(1+2*rank)*stacks)/100.0f*haste;
    assert(std::fabs(base*pct*haste-expected)<0.01f);
   }
  }
 }
 SpellInfo otherMode{};otherMode.Id=983869;config.mask=1;metadata.Apply(&otherMode);assert(otherMode.Effects[2].Effect==0);
 Player p; Skull skull{&p};
 skull.LimitCooldown(1); assert(!skull.prevented && p.cooldown[81101]==3000);
 p.now=2999; skull.LimitCooldown(1); assert(skull.prevented && p.cooldown[81101]==3000);
 p.now=3000; skull.prevented=false; skull.LimitCooldown(1); assert(!skull.prevented && p.cooldown[81101]==6000);
 p.area=false; p.cooldown.clear(); skull.LimitCooldown(1); assert(p.cooldown.empty());
 skull.caster=nullptr; skull.LimitCooldown(1);
 p.area=true; p.auras[983888]={30}; p.auras[983887]={1}; p.auras[983869]={28};
 Shot shot{&p}; shot.CaptureHoneIn(); shot.SpendHoneIn();
 assert(!p.GetAuraEffect(983888,0) && !p.GetAuraEffect(983887,0));
 shot.AddSniperTraining(); assert(shot.damage==1664);
 Shot next{&p}; next.CaptureHoneIn(); next.AddSniperTraining(); assert(next.damage==1280);
 p.area=false; p.auras[983888]={30}; Shot other{&p};
 other.CaptureHoneIn();other.SpendHoneIn();other.AddSniperTraining();
 assert(other.damage==1000 && p.GetAuraEffect(983888,0));
 assert(std::fabs(Area52Marksman::WithoutMunitionsHaste(0.8f*1.3f,-30)-0.8f)<0.00001f);
 assert(std::fabs(Area52Marksman::WithoutMunitionsHaste(0.8f/1.3f,30)-0.8f)<0.00001f);
 p.auras[81167]={5};p.auras[123]={1}; Cleanup cleanup{&p,81166};cleanup.Clear(nullptr,0);
 assert(!p.GetAuraEffect(81167,0) && p.GetAuraEffect(123,0));
 for(unsigned id:{983867u,983868u,983869u,983887u,983888u})p.auras[id]={1};
 cleanup.id=982890;cleanup.Clear(nullptr,0);
 for(unsigned id:{983867u,983868u,983869u,983887u,983888u})assert(!p.GetAuraEffect(id,0));
 cleanup.Clear(nullptr,0);assert(p.GetAuraEffect(123,0));
 AuraEffect dice{30,5171},otherHaste{20,1234};p.rangedEffects={&dice,&otherHaste};
 cleanup.id=979740;cleanup.RefreshSliceAndDice(nullptr,1);
 assert(dice.recalculated==1 && otherHaste.recalculated==0);
 cleanup.Clear(nullptr,1);assert(dice.recalculated==2 && otherHaste.recalculated==0);
 for(unsigned id:{954261u,954253u,954256u,954286u,954289u})p.auras[id]={1};
 cleanup.id=954252;cleanup.Clear(nullptr,0);
 for(unsigned id:{954261u,954253u,954256u,954286u,954289u})assert(!p.GetAuraEffect(id,0));
 for(unsigned rank=0;rank<3;++rank){
  p.auras[64418+rank]={4};p.auras[983867+rank]={4};
  cleanup.id=53302+rank;cleanup.Clear(nullptr,0);
  assert(!p.GetAuraEffect(64418+rank,0));assert(!p.GetAuraEffect(983867+rank,0));
  assert(p.GetAuraEffect(123,0));
 }
 p.area=true;p.stealth=true;manager.bonus.Effects[2].BasePoints=39;
 Shot stealthShot{&p};assert(stealthShot.Load());p.stealth=false;stealthShot.AddSniperTraining();assert(stealthShot.damage==1400);
 Shot exposed{&p};assert(exposed.Load());exposed.AddSniperTraining();assert(exposed.damage==1000);
 CastUnit caster;caster.auras[978716]={1};caster.auras[978717]={-30};caster.m_modAttackSpeedPct[2]=0.8f*1.3f;
 SpellInfo steady{};steady.Id=56641;int32 ms=2500;caster.CastTime(&steady,ms);assert(std::abs(ms-2000)<=1);
 steady.Id=999999;ms=2500;caster.CastTime(&steady,ms);assert(std::abs(ms-2000)<=1);
 steady.Id=19434;ms=2500;caster.CastTime(&steady,ms);assert(std::abs(ms-2600)<=1);
 caster.area=false;steady.Id=56641;ms=2500;caster.CastTime(&steady,ms);assert(std::abs(ms-2600)<=1);
 for(unsigned thunder:{275048u,275052u,275058u,1574554u,1574561u}){
  assert(Area52DragonWarrior::IsThunderSlam(thunder));
  assert(!Area52DragonWarrior::SlamAlwaysCrits(true,true,Area52DragonWarrior::IsThunderSlam(thunder)));
 }
 assert(Area52DragonWarrior::SlamAlwaysCrits(false,true,false));
}
'''
for token,text,signature in [('BODY_METADATA',dragon,'void OnLoadSpellCustomAttr'),('BODY_SKULL',dragon,'void LimitCooldown'),('BODY_CAPTURE',marksman,'void CaptureHoneIn'),('BODY_SPEND',marksman,'void SpendHoneIn'),('BODY_DAMAGE',marksman,'void AddSniperTraining')]:
 cpp=cpp.replace(token,body(text,signature))
cpp=cpp.replace('BODY_CLEANUP',body(marksman,'void Clear(AuraEffect const*'))
cpp=cpp.replace('BODY_REFRESH',body(marksman,'void RefreshSliceAndDice('))
cpp=cpp.replace('BODY_SHOT_LOAD',body(marksman[marksman.index('class spell_area52_syndicate_sniper_shot'):],'bool Load()'))
unit=(R.parent/'game/Entities/Unit/Unit.cpp').read_text()
cpp=cpp.replace('BODY_RANGED',body(unit[unit.index('void Unit::ModSpellCastTime'):],'case SPELL_DAMAGE_CLASS_RANGED:'))
player=(R.parent/'game/Entities/Player/Player.cpp').read_text()
origin='std::vector<uint32> Player::GetSpellInteractionOrigins(uint32 spellId) const '+body(player,'std::vector<uint32> Player::GetSpellInteractionOrigins')
cpp=cpp.replace('int main(){',origin+'\nint main(){')
with tempfile.TemporaryDirectory() as d:
 path=Path(d);(path/'test.cpp').write_text(cpp)
 subprocess.run(['g++','-std=c++17','-Wall','-Wextra','-Werror','-I',str(R),str(path/'test.cpp'),'-o',str(path/'test')],check=True)
 subprocess.run([str(path/'test')],check=True)
print('Extracted Skull Crusher cooldown and Hone In cast/consume/hit regressions passed; engine dispatch traced separately.')
