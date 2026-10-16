#include <algorithm>
#include <cassert>
#include <cstdint>
#include <memory>
#include <string>
#include <vector>
#include "AscensionPowerSurgeRules.h"

constexpr int CURRENT_CHANNELED_SPELL=0, CURRENT_GENERIC_SPELL=1;
constexpr int SPELL_STATE_FINISHED=3, POWER_MANA=0;
constexpr int SPELL_EFFECT_SCHOOL_DAMAGE=2, SPELL_EFFECT_HEALTH_LEECH=9, SPELL_AURA_PERIODIC_DAMAGE=3;
struct DataMap
{
    struct Base { virtual ~Base() = default; };
    std::unique_ptr<Base> value;
    template<class T> T* Get(std::string const&) { return dynamic_cast<T*>(value.get()); }
    template<class T> T* GetDefault(std::string const& key)
    {
        if (!value) value = std::make_unique<T>();
        return Get<T>(key);
    }
};
struct SpellInfo
{
    unsigned Id=1, SchoolMask=64;
    bool channel=false, harmful=true;
    bool IsChanneled() const { return channel; }
    bool HasEffect(int effect) const { return harmful && effect==SPELL_EFFECT_SCHOOL_DAMAGE; }
    bool HasAura(int) const { return false; }
};
struct Spell
{
    SpellInfo info;
    int state=0;
    int getState() const { return state; }
    SpellInfo const* GetSpellInfo() const { return &info; }
};
class Player;
class Unit
{
public:
    virtual ~Unit() = default;
    DataMap CustomData;
    bool alive=true;
    unsigned rank=0, mana=0, maximum=1000000, paid=0;
    bool HasAura(unsigned id) const { return rank && id==828573+rank; }
    bool IsAlive() const { return alive; }
    virtual Player* ToPlayer() { return nullptr; }
    unsigned GetPower(int) const { return mana; }
    unsigned GetMaxPower(int) const { return maximum; }
    void EnergizeBySpell(Unit*, unsigned, unsigned amount, int) { mana+=amount; paid+=amount; }
};
class Player : public Unit
{
public:
    Spell* channel=nullptr;
    Spell* generic=nullptr;
    bool applies=true;
    std::vector<unsigned> casts;
    Player* ToPlayer() override { return this; }
    Spell* GetCurrentSpell(int slot) { return slot==CURRENT_CHANNELED_SPELL ? channel : generic; }
    void CastSpell(Unit*, unsigned id, bool) { casts.push_back(id); }
};
namespace AscensionFreePick { bool Applies(Player* p) { return p->applies; } }
unsigned lastChance=0;
bool roll=true;
bool roll_chance_i(unsigned chance) { lastChance=chance; return roll; }

PRODUCTION_IMPLEMENTATION

int main()
{
    Unit attacker;
    Player owner;
    Spell spell;
    owner.generic=&spell;
    for(unsigned rank=1;rank<=3;++rank)
    {
        owner.rank=rank;
        for(unsigned school : {2u,4u,8u,16u,32u,64u})
        {
            spell.info.SchoolMask=school;
            AscensionPowerSurge::Damage(&owner,&attacker,1,0,true);
            assert(lastChance==2*rank);
            assert(owner.casts.back()==AscensionPowerSurge::Burst(school));
        }
    }
    auto count=owner.casts.size();
    AscensionPowerSurge::Damage(&owner,&attacker,1,0,false);
    AscensionPowerSurge::Damage(&owner,&attacker,0,0,true);
    AscensionPowerSurge::Damage(&owner,&owner,1,0,true);
    assert(owner.casts.size()==count);
    spell.info.harmful=false;
    AscensionPowerSurge::Damage(&owner,&attacker,1,0,true);
    assert(owner.casts.size()==count);
    owner.generic=nullptr;
    spell.info={986067,64,true,false};
    owner.channel=&spell;
    AscensionPowerSurge::Damage(&owner,&attacker,0,333,true);
    AscensionPowerSurge::Damage(&owner,&attacker,777,0,false);
    AscensionPowerSurge::Finish(&owner,986067,true);
    owner.channel=nullptr;
    AscensionPowerSurge::Update(&owner,999);
    assert(owner.paid==0);
    AscensionPowerSurge::Update(&owner,1);
    assert(owner.paid==111);
    AscensionPowerSurge::Update(&owner,9000);
    assert(owner.paid==1110);
    AscensionPowerSurge::Update(&owner,10000);
    assert(owner.paid==1110);
    owner.channel=&spell;
    AscensionPowerSurge::Damage(&owner,&attacker,1,6,false);
    AscensionPowerSurge::Finish(&owner,986067,true);
    AscensionPowerSurge::Update(&owner,10000);
    assert(owner.paid==1117);
    AscensionPowerSurge::Damage(&owner,&attacker,999,999,false);
    AscensionPowerSurge::Finish(&owner,986067,false);
    AscensionPowerSurge::Update(&owner,10000);
    assert(owner.paid==1117);
    AscensionPowerSurge::Damage(&owner,&attacker,500,500,false);
    AscensionPowerSurge::Finish(&owner,986067,true);
    owner.rank=2;
    AscensionPowerSurge::Update(&owner,1000);
    owner.rank=3;
    AscensionPowerSurge::Update(&owner,10000);
    assert(owner.paid==1117);
    AscensionPowerSurge::Damage(&owner,&attacker,500,500,false);
    AscensionPowerSurge::Finish(&owner,986067,true);
    owner.alive=false;
    AscensionPowerSurge::Update(&owner,1000);
    owner.alive=true;
    AscensionPowerSurge::Update(&owner,10000);
    assert(owner.paid==1117);
}
