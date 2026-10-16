from pathlib import Path
import json,struct,subprocess,tempfile
ROOT=Path(__file__).resolve().parents[3]
def function(text,name):
 start=text.index(name);begin=text.index('{',start);depth=1;end=begin+1
 while depth:
  depth+=(text[end]=='{')-(text[end]=='}');end+=1
 return text[start:end]
def main():
 raw=Path('/data/dbc/CharacterAdvancement.dbc').read_bytes();_,count,fields,size,strings=struct.unpack_from('<4s4I',raw)
 rows=[]
 for i in range(count):
  values=struct.unpack_from('<16I',raw,20+i*size)
  ranks=[v for v in values[5:14] if v]
  if values[15] and len(ranks)>1:rows.append((values[0],ranks))
 assert all(x in {r[0] for r in rows} for x in (427,432,739,855,1148))
 native=(ROOT/'src/server/coa/AscensionFreePick.cpp').read_text()
 player=(ROOT/'src/server/game/Entities/Player/Player.cpp').read_text()
 functions='\n'.join(function(native,name) for name in ('bool IsTalentRank(', 'bool CanLearnSpell(', 'bool SynchronizeTalentRanks('))
 restore=function(player,'void Player::learnSpell(')
 a=restore.index('    if (uint32 nextSpell');b=restore.index('    // xinef: if we learn new spell',a)
 restore=restore[a:b]
 removal=player[player.index('    if (itr->second->specMask == 0 && (AscensionFreePick::IsTalentRank'):]
 removal=removal[:removal.index('\n    {')].strip()[4:-1]
 cpp=r'''
#include <algorithm>
#include <cassert>
#include <cstdint>
#include <map>
#include <set>
#include <vector>
#include <string>
#include <iostream>
using uint32=std::uint32_t;
constexpr unsigned SPEC_MASK_ALL=255, PLAYERSPELL_REMOVED=2;
struct SpellInfo { unsigned Id; bool IsPassive() const { return true; } };
struct SpellMgr { std::map<unsigned,unsigned> next; std::map<unsigned,SpellInfo> info;
 unsigned GetNextSpellInChain(unsigned id) { return next[id]; }
 SpellInfo const* GetSpellInfo(unsigned id) { return &info.at(id); }
 bool IsAdditionalTalentSpell(unsigned) { return false; }
} manager;
auto sSpellMgr=&manager;
struct State { unsigned State=0,specMask=1; bool IsInSpec(unsigned) const {return specMask!=0;} };
using PlayerSpellMap=std::map<unsigned,State*>;
struct Player {
 bool freepick=true,baseline=false; unsigned m_activeSpec=0; PlayerSpellMap m_spells;
 std::set<unsigned> approved,auras; std::vector<unsigned> order;
 ~Player(){for(auto [id,state]:m_spells)delete state;}
 bool HasSpell(unsigned id) const {auto it=m_spells.find(id);return it!=m_spells.end()&&it->second->State!=PLAYERSPELL_REMOVED&&it->second->specMask;}
 bool HasAura(unsigned id) const {return auras.contains(id);}
 unsigned GetGUID() const {return 1;}
 bool IsNeedCastPassiveSpellAtLearn(SpellInfo const*) const {return true;}
 void RemoveAurasDueToSpell(unsigned id,unsigned){auras.erase(id);}
 void CastSpell(Player*,unsigned id,bool){auras.clear();auras.insert(id);}
 void removeSpell(unsigned,unsigned,bool);
 void learnSpell(unsigned,bool=false);
};
namespace AscensionFreePick {
std::set<unsigned> TalentSpells,ClassSpells;
std::vector<unsigned> SpecializationSpells;
struct Group {unsigned Parent;std::vector<unsigned> Children;};
std::vector<Group> LinkedSpellGroups;
bool Applies(Player const* p){return p->freepick;}
bool RacialSpell(Player const*,unsigned){return false;}
struct SpellLearningScope {static bool Allows(Player const* p,unsigned s){return p->approved.contains(s);}};
std::set<unsigned> KnownEntries(Player const* p){return p->approved;}
std::set<unsigned> SelectedSpells(std::set<unsigned> s){return s;}
std::set<unsigned> AuthorizedTalentSpells(Player const* p){return p->approved;}
FUNCTIONS
}
unsigned GetTalentSpellCost(unsigned id){return AscensionFreePick::TalentSpells.contains(id)?1:0;}
void Player::removeSpell(unsigned spell_id,unsigned,bool){
 auto itr=m_spells.find(spell_id);if(itr==m_spells.end()||!itr->second->specMask)return;
 itr->second->specMask=0;unsigned firstRankSpellId=spell_id;
 bool forget=baseline ? GetTalentSpellCost(firstRankSpellId)==0 : (REMOVAL);
 if(forget)itr->second->State=PLAYERSPELL_REMOVED;
 auras.erase(spell_id);
}
void Player::learnSpell(unsigned spellId,bool temporary){
 if(HasSpell(spellId))return;
 bool owned=m_spells.contains(spellId)&&m_spells[spellId]->State!=PLAYERSPELL_REMOVED;
 if(!(baseline&&owned)&&!AscensionFreePick::CanLearnSpell(this,spellId,temporary,owned))return;
 if(!m_spells.contains(spellId))m_spells[spellId]=new State;
 m_spells[spellId]->State=0;m_spells[spellId]->specMask=1;
 CastSpell(this,spellId,true);
 RESTORE
}
void select(Player& p,std::vector<unsigned> const& ranks,unsigned n){
 p.approved.clear();for(unsigned i=0;i<n;++i)p.approved.insert(ranks[i]);
 for(unsigned i=n;i<ranks.size();++i)p.removeSpell(ranks[i],255,false);
 for(unsigned i=0;i<n;++i)p.learnSpell(ranks[i]);
}
void check(Player const& p,std::vector<unsigned> const& ranks,unsigned n){
 for(unsigned i=0;i<ranks.size();++i)assert(p.HasSpell(ranks[i])==(i<n));
 if(n)assert(p.HasAura(ranks[n-1]));else assert(p.auras.empty());
}
int main(){
 std::vector<std::vector<unsigned>> catalog={CATALOG};
 unsigned cases=0;
 for(auto const& ranks:catalog){
  manager.next.clear();AscensionFreePick::TalentSpells={ranks.begin(),ranks.end()};AscensionFreePick::ClassSpells=AscensionFreePick::TalentSpells;
  for(unsigned i=0;i<ranks.size();++i){manager.info[ranks[i]]={ranks[i]};manager.next[ranks[i]]=i+1<ranks.size()?ranks[i+1]:0;}
  {Player old;old.baseline=true;select(old,ranks,ranks.size());select(old,ranks,0);select(old,ranks,1);assert(old.HasSpell(ranks.back()));}
  for(unsigned n=1;n<=ranks.size();++n){
   Player p;select(p,ranks,n);check(p,ranks,n);select(p,ranks,ranks.size());check(p,ranks,ranks.size());select(p,ranks,0);check(p,ranks,0);select(p,ranks,n);check(p,ranks,n);++cases;
   Player dirty;select(dirty,ranks,ranks.size());dirty.approved={ranks.begin(),ranks.begin()+n};
   AscensionFreePick::SynchronizeTalentRanks(&dirty);check(dirty,ranks,n);assert(!AscensionFreePick::SynchronizeTalentRanks(&dirty));
   for(auto id:ranks)dirty.learnSpell(id);check(dirty,ranks,n);
   Player saved;saved.approved={ranks.begin(),ranks.begin()+n};for(auto id:ranks){saved.m_spells[id]=new State;saved.m_spells[id]->specMask=0;}saved.learnSpell(ranks[0]);assert(!saved.HasSpell(ranks.back())||n==ranks.size());
  }
  Player granted;granted.approved.insert(ranks.back());granted.learnSpell(ranks.back());AscensionFreePick::SynchronizeTalentRanks(&granted);assert(granted.HasSpell(ranks.back()));
  Player normal;normal.freepick=false;assert(AscensionFreePick::CanLearnSpell(&normal,ranks.back(),false,true));
 }
 AscensionFreePick::TalentSpells.clear();AscensionFreePick::ClassSpells.clear();
 Player ability;ability.m_spells[999]=new State;assert(!AscensionFreePick::IsTalentRank(&ability,999));assert(AscensionFreePick::CanLearnSpell(&ability,999,false,true));
 std::cout<<"PASS catalog entries="<<catalog.size()<<" purchased-rank cases="<<cases<<" baseline defect reproduced for each chain"<<std::endl;
}
'''
 cpp=cpp.replace('FUNCTIONS',functions).replace('REMOVAL',removal).replace('RESTORE',restore).replace('CATALOG',','.join('{'+','.join(map(str,r))+'}' for _,r in rows))
 with tempfile.TemporaryDirectory() as tmp:
  p=Path(tmp);(p/'test.cpp').write_text(cpp)
  subprocess.run(['g++','-std=c++20','-Wall','-Wextra','-Werror','-Wno-misleading-indentation',str(p/'test.cpp'),'-o',str(p/'test')],check=True)
  subprocess.run([str(p/'test')],check=True)
 print('Scope: production learning guard, rank restoration fragment, removal predicate and cleanup function; mocked Player/aura storage. No gameplay.')
if __name__=='__main__':main()

