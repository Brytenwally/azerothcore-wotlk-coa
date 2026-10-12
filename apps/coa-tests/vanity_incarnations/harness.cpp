#include "AscensionIncarnationRules.h"
#include <array>
#include <cassert>
#include <cstdint>
#include <functional>
#include <memory>
#include <unordered_map>
#include <unordered_set>
#include <vector>
using uint32 = uint32_t;
using ObjectGuid = uint64_t;
constexpr int SPELL_AURA_TRANSFORM=1, SPELL_AURA_CLONE_CASTER=2, SPELL_AURA_MOD_SHAPESHIFT=3;
constexpr int SUMMON_PET=1;
namespace AscensionCompatConfig { constexpr int ENABLED=1; }
struct Config { template<class T> T GetConfigValue(int) { return T(enabled); } bool enabled=true; } ascensionCompatConfig;
struct Player;
struct Aura
{
    void SetDuration(int) { }
    void SetMaxDuration(int) { }
} visualAura;
struct AuraEffect { uint32 GetId() const { return 768; } } effect;
struct Unit
{
    virtual ~Unit() = default;
    virtual Player* ToPlayer() { return nullptr; }
    bool IsInWorld() const { return world; }
    bool IsAlive() const { return alive; }
    Unit* GetOwner() { return owner; }
    bool HasAuraType(int type) { return type==1 ? transform : type==2 && clone; }
    uint32 GetDisplayId() const { return display; }
    uint32 GetNativeDisplayId() const { return native; }
    float GetObjectScale() const { return scale; }
    void SetDisplayId(uint32 id, float value=1) { display=id;scale=value;++writes;if (hook) hook(this); }
    virtual void RestoreDisplayId() { SetDisplayId(native,scale); }
    bool world=true,alive=true,transform=false,clone=false;
    uint32 display=100,native=100,writes=0;
    float scale=1.25;
    Unit* owner=nullptr;
    std::function<void(Unit*)> hook;
};
struct Pet : Unit
{
    int getPetType() { return type; }
    uint32 GetEntry() { return entry; }
    ObjectGuid GetGUID() { return guid; }
    uint32 entry=416;
    ObjectGuid guid=2;
    int type=SUMMON_PET;
};
struct Player : Unit
{
    ObjectGuid GetGUID() { return 1; }
    bool HasAura(uint32 id, ObjectGuid) { return auras.contains(id); }
    void RemoveAurasDueToSpell(uint32 id, ObjectGuid) { auras.erase(id); }
    Aura* AddAura(uint32 id, Player*) { auras.insert(id);return &visualAura; }
    std::unordered_set<uint32> auras;
    Player* ToPlayer() override { return this; }
    Pet* GetPet() { return pet; }
    uint32 GetShapeshiftForm() { return form; }
    std::vector<AuraEffect*> const& GetAuraEffectsByType(int) { return forms; }
    uint32 GetModelForForm(uint32,uint32) { return baseForm; }
    void RestoreDisplayId() override { SetDisplayId(transform ? 999 : form ? baseForm : native,scale); }
    Pet* pet=nullptr;
    uint32 form=1,baseForm=200;
    std::vector<AuraEffect*> forms{&effect};
};
struct AppearanceInfo
{
    uint32 PrimaryCategory,SecondaryCategory,TertiaryCategory,CreatureDisplay;
    uint32 IncarnationSpell=0,CosmeticSpell=0;
};
struct PlayerCollectionState
{
    bool CanSeeSpellAppearances=true,UpdatingIncarnation=false;
    std::array<uint32,69> ActiveAppearances{};
    std::unordered_set<uint32> CollectedAppearances;
    std::unordered_set<uint32> AppliedCosmeticSpells;
    ObjectGuid IncarnationPet=0;
    uint32 IncarnationPetDisplay=0;
};
struct Service
{
    std::unordered_map<Player*,std::shared_ptr<PlayerCollectionState>> states;
    std::unordered_map<uint32,AppearanceInfo> _appearances;
    std::shared_ptr<PlayerCollectionState> GetState(Player* p) { auto i=states.find(p);return i==states.end()?nullptr:i->second; }
    // METHODS
};
int main()
{
    Service s;Player p,other;Pet pet;p.pet=&pet;pet.owner=&p;
    auto state=std::make_shared<PlayerCollectionState>();s.states[&p]=state;
    p.hook=pet.hook=[&](Unit* u){s.RefreshIncarnation(u);};
    for (uint32 form : {1u,2u,3u,4u,5u,8u,16u,22u,27u,29u,31u})
    {
        auto cat=AscensionIncarnations::FormCategory(form);assert(cat);
        auto id=1000+cat;auto model=2000+cat;
        s._appearances[id]={cat,0,0,model};state->ActiveAppearances[cat]=id;
        state->CollectedAppearances.insert(id);p.form=form;p.RestoreDisplayId();
        assert(p.display==model && p.native==100 && p.scale==1.25);
        state->CanSeeSpellAppearances=false;p.RestoreDisplayId();assert(p.display==p.baseForm);
        state->CanSeeSpellAppearances=true;p.transform=true;p.RestoreDisplayId();assert(p.display==999);
        p.transform=false;p.RestoreDisplayId();assert(p.display==model);
        state->ActiveAppearances[cat]=0;s.RefreshIncarnations(&p);assert(p.display==p.baseForm);
        state->ActiveAppearances[cat]=id;state->CollectedAppearances.erase(id);
        p.RestoreDisplayId();assert(p.display==p.baseForm);
        state->CollectedAppearances.insert(id);s._appearances[id].PrimaryCategory=68;
        p.RestoreDisplayId();assert(p.display==p.baseForm);
    }
    p.form=0;p.RestoreDisplayId();assert(p.display==p.native);
    for (uint32 entry : {416u,1100416u,1860u,1101860u,1863u,1101863u,417u,1100417u,17252u,1117252u})
    {
        auto cat=AscensionIncarnations::PetCategory(entry);assert(cat);
        auto id=1000+cat;auto model=2000+cat;
        s._appearances[id]={cat,0,0,model};state->ActiveAppearances[cat]=id;
        state->CollectedAppearances.insert(id);pet.entry=entry;pet.display=pet.native;
        s.RefreshIncarnation(&pet);assert(pet.display==model && pet.native==100);
        auto writes=pet.writes;s.RefreshIncarnation(&pet);assert(pet.writes==writes);
        pet.transform=true;pet.SetDisplayId(999);s.RefreshIncarnation(&pet);assert(pet.display==999);
        pet.transform=false;pet.RestoreDisplayId();assert(pet.display==model);
        state->ActiveAppearances[cat]=0;s.RefreshIncarnation(&pet);assert(pet.display==pet.native);
        state->ActiveAppearances[cat]=id;pet.owner=&other;s.RefreshIncarnation(&pet);assert(pet.display==pet.native);
        pet.owner=&p;++pet.guid;s.RefreshIncarnation(&pet);assert(pet.display==model);
        state->CanSeeSpellAppearances=false;s.RefreshIncarnation(&pet);assert(pet.display==pet.native);
        state->CanSeeSpellAppearances=true;
    }
    p.world=false;p.display=100;s.RefreshIncarnations(&p);assert(p.display==100);
    p.world=true;
    for (uint32 form : {31u,2u})
    {
        uint32 category=AscensionIncarnations::FormCategory(form);
        uint32 id=form==31 ? 47636 : 47647;
        uint32 spell=AscensionIncarnations::VisualSpell(id,category);
        assert(spell && !AscensionIncarnations::VisualSpell(id,18));
        s._appearances[id]={category,0,0,0,spell};
        state->ActiveAppearances[category]=id;state->CollectedAppearances.insert(id);
        p.form=form;p.RestoreDisplayId();assert(p.display==p.native && p.auras.contains(spell));
        p.transform=true;p.RestoreDisplayId();assert(p.display==999 && !p.auras.contains(spell));
        p.transform=false;p.RestoreDisplayId();assert(p.auras.contains(spell));
        p.clone=true;s.RefreshIncarnation(&p);assert(!p.auras.contains(spell));
        p.clone=false;p.RestoreDisplayId();assert(p.auras.contains(spell));
        state->CanSeeSpellAppearances=false;s.RefreshIncarnations(&p);
        assert(p.display==p.baseForm && !p.auras.contains(spell));
        state->CanSeeSpellAppearances=true;p.RestoreDisplayId();assert(p.auras.contains(spell));
        p.form=0;p.RestoreDisplayId();assert(!p.auras.contains(spell));
        p.form=form;p.RestoreDisplayId();state->ActiveAppearances[category]=0;s.RefreshIncarnations(&p);
        assert(p.display==p.baseForm && !p.auras.contains(spell));
    }
    assert(!AscensionIncarnations::FormCategory(0));assert(!AscensionIncarnations::PetCategory(1793));
    assert(!AscensionIncarnations::ModelCategory(30));assert(!AscensionIncarnations::ModelCategory(65));
}
