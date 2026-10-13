from pathlib import Path
import shutil
import subprocess
import tempfile

root = Path(__file__).resolve().parents[3]
source = root / 'src/server/coa/Area52WorldforgedAcquisition.cpp'
compiler = shutil.which('g++')
assert compiler, 'g++ is required for the native exchange fixture'
stub = r'''
#pragma once
#include <cassert>
#include <cstdint>
#include <map>
#include <vector>
using uint32 = std::uint32_t;
constexpr int NULL_BAG=0, NULL_SLOT=0;
enum InventoryResult { EQUIP_ERR_OK, EQUIP_ERR_INVENTORY_FULL };
using ItemPosCountVec=std::vector<int>;
struct SpellCastTargets {};
struct Item { uint32 entry=0; bool bound=false; uint32 GetEntry() const {return entry;}
    void SetBinding(bool value) {bound=value;} };
struct GameObject {uint32 entry=990100; uint32 GetEntry() const {return entry;}};
struct Player {
    std::map<uint32,uint32> bag;
    Item reward;
    bool alive=true, space=true, store=true, phase=true;
    uint32 cls=10;
    float distance=1;
    int saves=0, notices=0;
    bool IsAlive() const {return alive;}
    uint32 getClass() const {return cls;}
    bool HasItemCount(uint32 id,uint32 n,bool) {return bag[id]>=n;}
    void* GetSession() {return nullptr;}
    InventoryResult CanStoreNewItem(int,int,ItemPosCountVec&,uint32,uint32) {
        return space?EQUIP_ERR_OK:EQUIP_ERR_INVENTORY_FULL;}
    void SendEquipError(InventoryResult,void*,void*,uint32) {}
    Item* StoreNewItem(ItemPosCountVec&,uint32 id,bool) {
        if(!store) return nullptr;
        bag[id]++; reward={id,false}; return &reward;}
    void DestroyItemCount(uint32 id,uint32 n,bool) {assert(bag[id]>=n);bag[id]-=n;}
    void SaveInventoryAndGoldToDB(int) {saves++;}
    void SendNewItem(Item*,int,bool,bool) {notices++;}
    bool InSamePhase(GameObject*) {return phase;}
    float GetExactDist(GameObject*) {return distance;}
};
struct ChatHandler {ChatHandler(void*){} void SendSysMessage(char const*){}};
struct Database {int commits=0; int BeginTransaction(){return 1;}
    void CommitTransaction(int){commits++;}};
inline Database CharacterDatabase;
struct Config {bool enabled=true; template<class T>T GetOption(char const*,T){return enabled;}};
inline Config config;
inline Config* sConfigMgr=&config;
struct GameObjectScript {GameObjectScript(char const*){} virtual bool OnGossipHello(Player*,GameObject*)=0;};
struct ItemScript {ItemScript(char const*){} virtual bool OnUse(Player*,Item*,SpellCastTargets const&)=0;};
'''
checks = r'''
int main() {
    Player p; Cache cache; GameObject object;
    cache.OnGossipHello(&p,&object); assert(p.saves==0);
    p.bag[Key]=3; p.space=false;
    cache.OnGossipHello(&p,&object); assert(p.bag[Key]==3 && p.saves==0);
    p.space=true; p.store=false;
    cache.OnGossipHello(&p,&object); assert(p.bag[Key]==3 && p.saves==0);
    p.store=true; p.distance=10;
    cache.OnGossipHello(&p,&object); assert(p.bag[Key]==3);
    p.distance=1; p.phase=false;
    cache.OnGossipHello(&p,&object); assert(p.bag[Key]==3);
    p.phase=true; config.enabled=false;
    cache.OnGossipHello(&p,&object); assert(p.bag[Key]==3);
    config.enabled=true;
    cache.OnGossipHello(&p,&object); cache.OnGossipHello(&p,&object);
    assert(p.bag[Key]==1 && p.bag[100369]==2 && p.reward.bound && p.saves==2);
    Player other; other.bag[Key]=1;
    cache.OnGossipHello(&other,&object);
    assert(other.bag[Key]==0 && other.bag[100369]==1);
    Fragments fragments; Item fragment; fragment.entry=Fragment; SpellCastTargets targets;
    Player crafter; crafter.bag[Fragment]=4;
    fragments.OnUse(&crafter,&fragment,targets); assert(crafter.bag[Fragment]==4 && !crafter.saves);
    crafter.bag[Fragment]=10; crafter.space=false;
    fragments.OnUse(&crafter,&fragment,targets); assert(crafter.bag[Fragment]==10);
    crafter.space=true;
    fragments.OnUse(&crafter,&fragment,targets); fragments.OnUse(&crafter,&fragment,targets);
    assert(crafter.bag[Fragment]==0 && crafter.bag[Key]==2 && crafter.saves==2);
    Player nonhero; nonhero.cls=1; nonhero.bag[Key]=1;
    cache.OnGossipHello(&nonhero,&object); assert(nonhero.bag[Key]==1 && !nonhero.saves);
    assert(CharacterDatabase.commits==5);
}
'''
with tempfile.TemporaryDirectory() as temporary:
    folder = Path(temporary)
    (folder / 'stub.h').write_text(stub)
    for header in ('Chat','Config','DatabaseEnv','GameObject','Item','Player','ScriptMgr'):
        (folder / (header+'.h')).write_text('#include "stub.h"\n')
    unit = folder / 'test.cpp'
    unit.write_text(source.read_text() + checks)
    binary = folder / 'test'
    subprocess.run([compiler,'-std=c++20','-Wall','-Wextra','-Werror','-I',str(folder),str(unit),'-o',str(binary)],check=True)
    subprocess.run([str(binary)],check=True)
print('Actual exchange handler passed mocked inventory, repeated acquisition, player isolation and failure checks')
