/* Copyright (C) 2016+ AzerothCore, GNU AGPL v3. */
#ifndef ASCENSION_FREE_PICK_H
#define ASCENSION_FREE_PICK_H

#include "AscensionCoATalentState.h"
#include <array>
#include <map>
#include <set>
#include <string>

class Player;
namespace Area52Build { struct Entry; }
struct AscensionClientConfig;

namespace AscensionFreePick
{
using KnownEntry = AscensionCoATalentState::KnownEntry;

struct Entry
{
    std::uint32_t Id = 0;
    std::vector<std::uint32_t> Spells;
    std::array<std::uint32_t, 3> Required{};
    std::array<std::uint32_t, 3> Masteries{};
    std::vector<std::uint32_t> Connected;
    std::array<std::uint32_t, 6> Investment{};
    std::uint32_t Level = 0;
    std::uint32_t AE = 0;
    std::uint32_t TE = 0;
    std::uint32_t Quality = 0;
    std::uint32_t QualityCost = 0;
    std::uint32_t Class = 0;
    std::uint32_t Tab = 0;
    std::uint32_t Group = 0;
    std::uint32_t Flags = 0;
    std::uint32_t Points = 0;
    std::uint32_t RequiredPoints = 0;
};

struct Budget
{
    std::uint32_t AE = 0;
    std::uint32_t TE = 0;
};

struct Catalog
{
    std::map<std::uint32_t, Entry> Entries;
    std::map<std::uint32_t, Budget> Budgets;
};

struct Result
{
    char const* Status = "CA_UPDATE_ENTRIES_OK";
    char const* Learn = "";
    std::uint32_t EntryId = 0;
    std::uint32_t Rank = 0;
    explicit operator bool() const;
};

inline constexpr std::array<std::uint32_t, 4> QualityLimits = { 10, 12, 11, 6 };
inline constexpr std::uint32_t QualityMode = 0x400;

bool LoadCatalog(Catalog& catalog);
std::vector<KnownEntry> IncludeJudgementMastery(std::vector<KnownEntry> wanted);
Result Validate(Catalog const& catalog, std::uint32_t level, std::vector<KnownEntry> const& wanted,
    std::vector<KnownEntry> const& previous = {});
bool SameSelection(std::vector<KnownEntry> const& left, std::vector<KnownEntry> const& right);
void Initialize(bool enabled);
bool Enabled();
inline constexpr std::array<std::uint32_t, 20> SpecializationSpells = { 979993, 979994, 979995,
    979996, 979997, 979986, 979987, 979988, 84874, 84876, 84878, 84880, 84882, 84884, 84886, 84888,
    84890, 84892, 84894, 84896 };
std::uint32_t ActiveSpecialization(Player const* player);
bool SwitchSpecialization(Player* player, std::uint32_t index, std::string& error);
void RegisterSpecializations();
std::uint32_t Investment(Player const* player, std::uint32_t classId, std::uint32_t tabId, bool talents);
bool Applies(Player const* player);
class SpellLearningScope
{
public:
    SpellLearningScope(Player const* player, std::set<std::uint32_t> const& spells);
    ~SpellLearningScope();
    SpellLearningScope(SpellLearningScope const&) = delete;
    SpellLearningScope& operator=(SpellLearningScope const&) = delete;
    static bool Allows(Player const* player, std::uint32_t spell);
private:
    Player const* Owner;
    std::set<std::uint32_t> const& Spells;
    SpellLearningScope const* Previous;
    static thread_local SpellLearningScope const* Current;
};
bool IsTalentRank(Player const* player, std::uint32_t spell);
bool CanLearnSpell(Player const* player, std::uint32_t spell, bool temporary, bool alreadyOwned);
bool SynchronizeLinkedSpells(Player* player);
bool CanTrainCompanion(Player const* player, std::uint32_t spell);
bool LearnCompanion(Player* player, std::uint32_t spell);
bool RemoveQuestGrantedClassSpells(Player* player);
bool IsLinkedSpellParent(std::uint32_t spell);
struct RankUpgrade
{
    std::uint32_t Spell;
    std::uint32_t Previous;
    std::uint32_t Level;
};
std::vector<RankUpgrade> RankUpgrades(Player const* player);
bool LearnRankUpgrade(Player* player, std::uint32_t spell, bool save = true);
std::uint32_t LearnAvailableRanks(Player* player);
void AppendClientConfig(AscensionClientConfig& config);
std::vector<KnownEntry> KnownEntries(Player const* player);
Result Apply(Player* player, std::vector<KnownEntry> const& wanted, bool chargeIndividualRemoval = true,
    bool specializationChange = false);
Result PlanMasteryRemoval(Catalog const& catalog, std::vector<KnownEntry> const& current,
    std::uint32_t entryId, std::vector<KnownEntry>& wanted);
Result RemoveMasteryEntry(Player* player, std::uint32_t entryId);
Result Purge(Player* player, bool talents);
Result PlanSelection(Player const* player, Area52Build::Entry const& plan, std::vector<KnownEntry>& wanted,
    bool retainExisting = false, std::uint32_t maximumLevel = 80, bool learnSpells = true);
}

#endif
