/* Copyright (C) 2016+ AzerothCore, GNU AGPL v3. */
#include "AscensionFreePick.h"
#include "AscensionPrestigeReset.h"
#include "AscensionSpecialization.h"
#include "AscensionMysticEnchant.h"
#include "AscensionRemovedSpellActionBars.h"
#include "AscensionFreePickLinkedSpells.h"
#include "Area52CompanionTraining.h"
#include "Area52BuildSelection.h"
#include "Area52ActiveBuild.h"
#include "AscensionMysticEnchantCatalog.h"
#include "AscensionMysticSpellGrants.h"
#include "AscensionCoAConfig.h"
#include "AscensionRacialAbilities.h"
#include "DBCStores.h"
#include "ObjectMgr.h"
#include "QuestDef.h"
#include "Log.h"
#include "Player.h"
#include "Item.h"
#include "ScriptMgr.h"
#include "SpellInfo.h"
#include "Spell.h"
#include "Random.h"
#include "SpellScript.h"
#include "Chat.h"
#include "Pet.h"
#include "WorldPacket.h"
#include "SpellAuras.h"
#include "SpellMgr.h"
#include "WorldSession.h"
#include <algorithm>
#include <chrono>
#include <set>

void RefreshArea52DruidControlContracts();

namespace AscensionFreePick
{
namespace
{
Catalog Data;
bool Active = false;
std::set<std::uint32_t> ClassSpells;
std::set<uint32> TalentSpells;
constexpr char SelectionSetting[] = "core.ascension_freepick_entries";

bool RacialSpell(Player const* player, uint32 spell)
{
    for (auto const& skill : AscensionRacialAbilities::Skills)
        if (skill.RaceId == player->getRace())
            for (auto const* ability : GetSkillLineAbilitiesBySkillLine(skill.SkillId))
                if (ability->Spell == spell &&
                    AscensionRacialAbilities::CanLearn(*ability, player->getRace(), player->getClass()))
                    return true;
    return false;
}

void ExpandGrantedSpells(std::set<uint32>& spells, bool ranks, bool triggers = false)
{
    std::vector<uint32> pending(spells.begin(), spells.end());
    for (std::size_t i = 0; i < pending.size(); ++i)
    {
        auto add = [&](uint32 spell)
        {
            if (spell && spells.insert(spell).second)
                pending.push_back(spell);
        };
        uint32 const spell = pending[i];
        if (ranks)
        {
            add(sSpellMgr->GetFirstSpellInChain(spell));
            add(sSpellMgr->GetNextSpellInChain(spell));
        }
        if (auto const* info = sSpellMgr->GetSpellInfo(spell))
            for (auto const& effect : info->GetEffects())
                if (effect.IsEffect(SPELL_EFFECT_LEARN_SPELL) ||
                    (triggers && effect.IsEffect(SPELL_EFFECT_TRIGGER_SPELL)))
                    add(effect.TriggerSpell);
        for (auto const& group : LinkedSpellGroups)
            if (group.Parent == spell)
                for (auto child : group.Children)
                    add(child);
    }
}

void ExpandBuffSources(std::set<uint32>& spells)
{
    ExpandGrantedSpells(spells, true);
    std::vector<uint32> pending(spells.begin(), spells.end());
    for (std::size_t i = 0; i < pending.size(); ++i)
    {
        auto add = [&](uint32 spell)
        {
            if (spell && spells.insert(spell).second)
                pending.push_back(spell);
        };
        if (auto const* info = sSpellMgr->GetSpellInfo(pending[i]))
            for (auto const& effect : info->GetEffects())
                add(effect.TriggerSpell);
        for (auto type : {SPELL_LINK_CAST, SPELL_LINK_HIT, SPELL_LINK_AURA})
            if (auto const* linked = sSpellMgr->GetSpellLinked(int32(pending[i]) + type))
                for (auto spell : *linked)
                    if (spell > 0)
                        add(uint32(spell));
    }
}

void RemoveSelectionBuffs(Player* player, std::set<uint32> removed, std::set<uint32> retained)
{
    for (auto const& grant : AscensionMysticEnchant::SpellGrants)
        if (AscensionMysticEnchant::GrantsActive(player, grant.Enchant))
            retained.insert(grant.Spell);
    ExpandBuffSources(removed);
    ExpandBuffSources(retained);
    std::set<uint32> cleanup;
    bool changed;
    do
    {
        changed = false;
        for (auto const& [id, application] : player->GetAppliedAuras())
        {
            auto const* aura = application->GetBase();
            if (aura->GetCasterGUID() != player->GetGUID() || retained.contains(id))
                continue;
            auto const* source = aura->GetTriggeredByAuraSpellInfo();
            if (removed.contains(id) || (source && removed.contains(source->Id)
                && !retained.contains(source->Id)))
            {
                cleanup.insert(id);
                changed = removed.insert(id).second || changed;
            }
        }
    } while (changed);
    for (auto spell : cleanup)
        player->RemoveAurasDueToSpell(spell, player->GetGUID());
}

std::set<std::uint32_t> SelectedSpells(std::vector<KnownEntry> const& entries)
{
    std::set<std::uint32_t> spells;
    for (auto const& pick : entries)
        if (auto const found = Data.Entries.find(pick.EntryId); found != Data.Entries.end())
            for (std::size_t rank = 0; rank < std::min<std::size_t>(pick.Rank, found->second.Spells.size()); ++rank)
                spells.insert(found->second.Spells[rank]);
    for (auto const& group : LinkedSpellGroups)
        if (spells.contains(group.Parent))
            for (auto child : group.Children)
                if (child)
                    spells.insert(child);
    ExpandGrantedSpells(spells, false);
    return spells;
}
}

void Initialize(bool enabled)
{
    Active = enabled && LoadCatalog(Data);
    sSpellMgr->RefreshArea52DanceOfDeathProc();
    RefreshArea52DruidControlContracts();
    ClassSpells.clear();
    TalentSpells.clear();
    if (Active)
    {
        for (auto const& [id, entry] : Data.Entries)
            for (auto spell : entry.Spells)
            {
                ClassSpells.insert(spell);
                if (entry.TE)
                    TalentSpells.insert(spell);
                std::set<std::uint32_t> visited;
                for (auto rank = sSpellMgr->GetFirstSpellInChain(spell); rank && visited.insert(rank).second;
                    rank = sSpellMgr->GetNextSpellInChain(rank))
                    ClassSpells.insert(rank);
            }
        for (uint32 i = 0; i < sSkillLineAbilityStore.GetNumRows(); ++i)
            if (auto const* ability = sSkillLineAbilityStore.LookupEntry(i); ability && ability->ClassMask)
                if (auto const* skill = sSkillLineStore.LookupEntry(ability->SkillLine);
                    skill && skill->categoryId == SKILL_CATEGORY_CLASS)
                    ClassSpells.insert(ability->Spell);
        ExpandGrantedSpells(ClassSpells, true);
    }
    if (enabled)
        LOG_INFO("coa", "Free Pick advancement: enabled={}, entries={}, essence levels={}",
            Active, Data.Entries.size(), Data.Budgets.size());
}

bool Enabled()
{
    return Active;
}

bool Applies(Player const* player)
{
    if (!Active || !player || player->getClass() != CLASS_HERO)
        return false;
    auto const mask = sScriptMgr->OnPlayerGetGameModeMask(player);
    return !mask || !(*mask & ~QualityMode);
}

bool IsTalentRank(Player const* player, std::uint32_t spell)
{
    return Applies(player) && TalentSpells.contains(spell);
}

std::set<std::uint32_t> AuthorizedTalentSpells(Player const* player)
{
    auto spells = SelectedSpells(KnownEntries(player));
    for (auto const& grant : AscensionMysticEnchant::SpellGrants)
        if (AscensionMysticEnchant::GrantsActive(player, grant.Enchant))
            spells.insert(grant.Spell);
    ExpandGrantedSpells(spells, false);
    return spells;
}

bool SynchronizeTalentRanks(Player* player)
{
    if (!Applies(player) || !player->IsInWorld())
        return false;
    auto const approved = AuthorizedTalentSpells(player);
    bool changed = false;
    for (auto spell : TalentSpells)
        if (!approved.contains(spell) && (player->HasSpell(spell) || player->HasAura(spell)))
        {
            player->removeSpell(spell, SPEC_MASK_ALL, false);
            player->RemoveAurasDueToSpell(spell, player->GetGUID());
            changed = true;
        }
    for (auto spell : TalentSpells)
        if (approved.contains(spell) && !player->HasSpell(spell))
        {
            player->learnSpell(spell);
            changed |= player->HasSpell(spell);
        }
    for (auto spell : TalentSpells)
        if (approved.contains(spell) && player->HasSpell(spell))
            if (auto const* info = sSpellMgr->GetSpellInfo(spell); info && info->IsPassive() && info->HasAnyAura()
                && player->IsNeedCastPassiveSpellAtLearn(info) && !player->HasAura(spell))
            {
                auto next = sSpellMgr->GetNextSpellInChain(spell);
                if (!next || !approved.contains(next))
                {
                    player->CastSpell(player, spell, true);
                    changed |= player->HasAura(spell);
                }
            }
    return changed;
}

bool CanLearnSpell(Player const* player, std::uint32_t spell, bool temporary, bool alreadyOwned)
{
    if (IsTalentRank(player, spell))
        return AuthorizedTalentSpells(player).contains(spell);
    if (Applies(player) && (spell == 6991 || spell == 1106991 || spell == 1356991))
        return false;
    bool const companion = std::any_of(LinkedSpellGroups.begin(), LinkedSpellGroups.end(),
        [spell](auto const& group)
        {
            return ClassSpells.contains(group.Parent) &&
                std::find(group.Children.begin(), group.Children.end(), spell) != group.Children.end();
        });
    return !Applies(player) || alreadyOwned || RacialSpell(player, spell) ||
        std::find(SpecializationSpells.begin(), SpecializationSpells.end(), spell) != SpecializationSpells.end() ||
        (!ClassSpells.contains(spell) && !companion) ||
        SpellLearningScope::Allows(player, spell) ||
        (temporary && SelectedSpells(KnownEntries(player)).contains(spell));
}

bool RemoveQuestGrantedClassSpells(Player* player)
{
    if (!Applies(player))
        return false;
    std::set<uint32> approved = SelectedSpells(KnownEntries(player));
    for (auto const& grant : AscensionMysticEnchant::SpellGrants)
        if (AscensionMysticEnchant::GrantsActive(player, grant.Enchant))
            approved.insert(grant.Spell);
    ExpandGrantedSpells(approved, true);
    std::set<uint32> rewards;
    for (uint32 questId : player->getRewardedQuests())
        if (auto const* quest = sObjectMgr->GetQuestTemplate(questId))
        {
            uint32 const reward = quest->GetRewSpellCast() > 0 ? uint32(quest->GetRewSpellCast()) : quest->GetRewSpell();
            if (reward)
                rewards.insert(reward);
        }
    ExpandGrantedSpells(rewards, true, true);
    std::vector<uint32> remove;
    for (uint32 spell : rewards)
        if (ClassSpells.contains(spell) && !approved.contains(spell) && !RacialSpell(player, spell) && player->HasSpell(spell))
        {
            auto const* owners = player->FindPlayerSettings("core.freepick.mystic.grant." + std::to_string(spell));
            bool const enchanted = owners && std::any_of(owners->begin() + std::min<std::size_t>(1, owners->size()),
                owners->end(), [&](auto const& owner)
                {
                    return owner.value && AscensionMysticEnchant::GrantsActive(player, owner.value);
                });
            if (!enchanted)
                remove.push_back(spell);
        }
    for (uint32 spell : remove)
        player->removeSpell(spell, SPEC_MASK_ALL, false);
    return !remove.empty();
}

bool IsLinkedSpellParent(std::uint32_t spell)
{
    return spell == Area52CompanionTraining::Mastery || spell == Area52CompanionTraining::Runeforging || std::any_of(LinkedSpellGroups.begin(), LinkedSpellGroups.end(),
        [spell](auto const& group) { return group.Parent == spell; });
}

bool CanTrainCompanion(Player const* player, std::uint32_t spell)
{
    uint32 const parent = Area52CompanionTraining::Parent(spell);
    return Applies(player) && parent && player->HasSpell(parent);
}

bool LearnCompanion(Player* player, std::uint32_t spell)
{
    if (!CanTrainCompanion(player, spell) || player->HasSpell(spell) || !sSpellMgr->GetSpellInfo(spell))
        return false;
    std::set<std::uint32_t> const allowed = {spell};
    SpellLearningScope learning(player, allowed);
    player->learnSpell(spell);
    if (!player->HasSpell(spell))
        return false;
    player->UpdatePlayerSetting("core.freepick.companion." + std::to_string(spell), 0, 1);
    return true;
}

bool SynchronizeLinkedSpells(Player* player)
{
    if (!Applies(player) || !player->IsInWorld() || !player->GetSession() || player->GetSession()->PlayerLogout())
        return false;
    std::set<std::uint32_t> wanted, managed;
    for (auto const& pick : KnownEntries(player))
        if (auto const found = Data.Entries.find(pick.EntryId); found != Data.Entries.end())
            for (std::size_t rank = 0; rank < std::min<std::size_t>(pick.Rank, found->second.Spells.size()); ++rank)
                wanted.insert(found->second.Spells[rank]);
    for (auto const& group : LinkedSpellGroups)
        if (ClassSpells.contains(group.Parent))
            for (auto child : group.Children)
                if (child && sSpellMgr->GetSpellInfo(child))
                {
                    managed.insert(child);
                    if (player->HasSpell(group.Parent))
                        wanted.insert(child);
                }
    std::vector<uint32> lessons(Area52CompanionTraining::Spells.begin(), Area52CompanionTraining::Spells.end());
    lessons.insert(lessons.end(), Area52CompanionTraining::Runes.begin(), Area52CompanionTraining::Runes.end());
    for (auto spell : lessons)
        if (player->GetPlayerSetting("core.freepick.companion." + std::to_string(spell), 0).value == 1)
        {
            managed.insert(spell);
            if (player->HasSpell(Area52CompanionTraining::Parent(spell)))
                wanted.insert(spell);
        }
    bool changed = SynchronizeTalentRanks(player);
    SpellLearningScope learning(player, wanted);
    for (auto child : managed)
    {
        bool const known = player->HasSpell(child);
        std::string const source = "core.freepick.companion." + std::to_string(child);
        auto const ownership = player->GetPlayerSetting(source, 0).value;
        if (wanted.contains(child))
        {
            if (!ownership)
                player->UpdatePlayerSetting(source, 0, known ? 2 : 1);
            if (!known)
                player->learnSpell(child);
        }
        else if (ownership == 1 && known)
            player->removeSpell(child, SPEC_MASK_ALL, false);
        changed |= known != player->HasSpell(child);
    }
    return changed;
}

void AppendClientConfig(AscensionClientConfig& config)
{
    if (!Active)
        return;
    constexpr char enabled[] = "CONFIG_CHARACTER_ADVANCEMENT_QUALITIES_ENABLED";
    std::erase_if(config.Booleans, [](auto const& pair)
        { return pair.first == "CONFIG_CHARACTER_ADVANCEMENT_QUALITIES_ENABLED"; });
    config.Booleans.emplace_back(enabled, true);
    constexpr char purge[] = "CONFIG_CHARACTER_ADVANCEMENT_HERO_ABILITY_TALENT_PURGE_ENABLED";
    std::erase_if(config.Booleans, [](auto const& pair)
        { return pair.first == "CONFIG_CHARACTER_ADVANCEMENT_HERO_ABILITY_TALENT_PURGE_ENABLED"; });
    config.Booleans.emplace_back(purge, true);
    config.Booleans.emplace_back("CONFIG_AREA52_FLAT_RUNE_RESETS", true);
    config.Booleans.emplace_back("CONFIG_AREA52_FREE_RESETS", true);
    constexpr std::array<char const*, 4> qualities = { "UNCOMMON", "RARE", "EPIC", "LEGENDARY" };
    for (char const* expansion : { "CLASSIC", "THE_BURNING_CRUSADE", "WRATH_OF_THE_LICH_KING" })
        for (std::size_t i = 0; i < qualities.size(); ++i)
        {
            std::string const key = std::string("CONFIG_CHARACTER_ADVANCEMENT_QUALITIES_MAX_") +
                qualities[i] + "_ABILITIES_" + expansion;
            std::erase_if(config.Integers, [&](auto const& pair) { return pair.first == key; });
            config.Integers.emplace_back(key, QualityLimits[i]);
        }
}

std::vector<KnownEntry> KnownEntries(Player const* player)
{
    std::vector<KnownEntry> entries;
    if (auto const* settings = player->FindPlayerSettings(SelectionSetting))
        for (auto const& setting : *settings)
        {
            std::uint32_t const id = setting.value / 10;
            std::uint32_t const rank = setting.value % 10;
            auto const found = Data.Entries.find(id);
            if (found == Data.Entries.end() || !rank || rank > found->second.Spells.size())
                continue;
            entries.push_back({ id, rank, rank, false, std::uint32_t(entries.size()) });
        }
    return entries;
}

std::uint32_t Investment(Player const* player, std::uint32_t classId, std::uint32_t tabId, bool talents)
{
    std::uint32_t total = 0;
    for (auto const& pick : KnownEntries(player))
    {
        auto const& entry = Data.Entries.at(pick.EntryId);
        if (classId == 1 || (entry.Class == classId && (tabId == 1 || entry.Tab == tabId)))
            total += (talents ? entry.TE : entry.AE) * pick.Rank;
    }
    return total;
}

Result PlanSelection(Player const* player, Area52Build::Entry const& plan, std::vector<KnownEntry>& wanted,
    bool retainExisting, std::uint32_t maximumLevel, bool learnSpells)
{
    if (!Applies(player))
        return { "CA_UPDATE_ENTRIES_UNKNOWN" };
    return Area52Build::Select(Data, plan, player->GetLevel(), wanted, KnownEntries(player), retainExisting, maximumLevel, learnSpells);
}

Result Apply(Player* player, std::vector<KnownEntry> const& requested, bool, bool specializationChange)
{
    auto const wanted = IncludeJudgementMastery(requested);
    if (!Applies(player) || !player->IsInWorld())
        return { "CA_UPDATE_ENTRIES_UNKNOWN" };
    if (!player->IsAlive())
        return { "CA_UPDATE_ENTRIES_NOT_TRAVERSIBLE", "CA_LEARN_NOT_WHILE_DEAD" };
    if (player->IsInCombat())
        return { "CA_UPDATE_ENTRIES_NOT_TRAVERSIBLE", "CA_LEARN_NOT_IN_COMBAT" };
    if (player->InBattleground() || player->InArena())
        return { "CA_UPDATE_ENTRIES_NOT_TRAVERSIBLE", "CA_LEARN_NOT_IN_BATTLEGROUNDS" };
    auto const before = KnownEntries(player);
    Result const validated = Validate(Data, player->GetLevel(), wanted, specializationChange ? wanted : before);
    if (!validated)
        return validated;
    auto const oldSpells = SelectedSpells(before);
    auto const newSpells = SelectedSpells(wanted);
    SpellLearningScope learning(player, newSpells);
    for (auto const spell : newSpells)
        if (!sSpellMgr->GetSpellInfo(spell))
            return { "CA_UPDATE_ENTRIES_BAD_ENTRY", "CA_LEARN_BAD_ABILITY" };

    if (SameSelection(before, wanted))
    {
        if (SynchronizeTalentRanks(player) && !specializationChange)
            player->SaveToDB(false, false);
        return {};
    }
    auto const actionSpells = AscensionRemovedSpellActionBars::Capture(player);
    RemoveSelectionBuffs(player, oldSpells, newSpells);
    auto const* settings = player->FindPlayerSettings(SelectionSetting);
    std::size_t const oldSize = settings ? settings->size() : 0;
    for (std::size_t index = 0; index < std::max(oldSize, wanted.size()); ++index)
        player->UpdatePlayerSetting(SelectionSetting, std::uint32_t(index), index < wanted.size() ?
            wanted[index].EntryId * 10 + wanted[index].Rank : 0);
    std::set<std::uint32_t> retainedRoots;
    for (auto spell : newSpells)
        retainedRoots.insert(sSpellMgr->GetFirstSpellInChain(spell));
    for (auto spell : oldSpells)
        if (!retainedRoots.contains(sSpellMgr->GetFirstSpellInChain(spell)))
        {
            std::set<std::uint32_t> visited;
            for (auto rank = sSpellMgr->GetNextSpellInChain(spell); rank && visited.insert(rank).second;
                rank = sSpellMgr->GetNextSpellInChain(rank))
                player->removeSpell(rank, SPEC_MASK_ALL, false);
        }
    for (auto const spell : oldSpells)
        if (!newSpells.count(spell))
            player->removeSpell(spell, SPEC_MASK_ALL, false);
    for (auto const& pick : wanted)
        for (std::size_t rank = 0; rank < pick.Rank; ++rank)
            player->learnSpell(Data.Entries.at(pick.EntryId).Spells[rank]);
    SynchronizeLinkedSpells(player);
    AscensionMysticEnchant::RefreshEquipped(player);
    AscensionRemovedSpellActionBars::Reconcile(player, actionSpells);
    if (!specializationChange)
        player->SaveToDB(false, false);
    return {};
}

Result RemoveMasteryEntry(Player* player, std::uint32_t entryId)
{
    if (!Applies(player) || !player->IsInWorld())
        return { "CA_UPDATE_ENTRIES_UNKNOWN" };
    std::vector<KnownEntry> wanted;
    auto const planned = PlanMasteryRemoval(Data, KnownEntries(player), entryId, wanted);
    if (!planned)
        return planned;
    return Apply(player, wanted);
}

Result Purge(Player* player, bool talents)
{
    if (!Applies(player) || !player->IsInWorld())
        return { "CA_UPDATE_ENTRIES_UNKNOWN" };
    auto wanted = KnownEntries(player);
    auto const before = wanted.size();
    std::erase_if(wanted, [&](KnownEntry const& pick)
        { return talents ? Data.Entries.at(pick.EntryId).TE != 0 : Data.Entries.at(pick.EntryId).AE != 0; });
    if (before == wanted.size())
        return { "CA_UPDATE_ENTRIES_BAD_ENTRY" };
    auto const result = Apply(player, wanted, false);
    if (!result)
        return result;
    Area52Build::PauseAutoLearnForReset(player);
    player->SaveToDB(false, false);
    return {};
}

std::vector<RankUpgrade> RankUpgrades(Player const* player)
{
    std::vector<RankUpgrade> upgrades;
    if (!Applies(player))
        return upgrades;
    std::set<uint32> roots;
    for (auto const& selected : KnownEntries(player))
    {
        auto const found = Data.Entries.find(selected.EntryId);
        if (found == Data.Entries.end())
            continue;
        auto const& entry = found->second;
        for (std::size_t index = 0; index < selected.Rank && index < entry.Spells.size(); ++index)
        {
            uint32 const spell = entry.Spells[index];
            std::set<uint32> granted = {spell};
            ExpandGrantedSpells(granted, false);
            if (entry.TE || !entry.AE)
                granted.erase(spell);
            roots.insert(granted.begin(), granted.end());
        }
    }
    for (auto const& grant : AscensionMysticEnchant::SpellGrants)
        if (AscensionMysticEnchant::GrantsActive(player, grant.Enchant))
            roots.insert(grant.Spell);
    std::set<uint32> offered;
    for (uint32 root : roots)
    {
        if (!player->HasSpell(root))
            continue;
        auto previous = root;
        std::set<uint32> visited;
        for (auto rank = sSpellMgr->GetNextSpellInChain(previous); rank && visited.insert(rank).second;
            previous = rank, rank = sSpellMgr->GetNextSpellInChain(rank))
            if (auto const* info = sSpellMgr->GetSpellInfo(rank); info && offered.insert(rank).second)
            {
                SpellEntry const* client = sSpellStore.LookupEntry(rank);
                uint32 const level = client ? std::max(client->BaseLevel, client->SpellLevel) :
                    std::max(info->BaseLevel, info->SpellLevel);
                upgrades.push_back({rank, previous, level});
            }
    }
    return upgrades;
}

bool LearnRankUpgrade(Player* player, std::uint32_t spell, bool save)
{
    if (!Applies(player) || !player->IsInWorld() || !player->IsAlive())
        return false;
    for (auto const& rank : RankUpgrades(player))
        if (rank.Spell == spell && player->GetLevel() >= rank.Level &&
            player->HasSpell(rank.Previous) && !player->HasSpell(spell))
        {
            std::set<std::uint32_t> const authorized = {spell};
            SpellLearningScope learning(player, authorized);
            bool managed = false;
            for (auto const& grant : AscensionMysticEnchant::SpellGrants)
                if (sSpellMgr->GetFirstSpellInChain(spell) == grant.Spell &&
                    AscensionMysticEnchant::GrantsActive(player, grant.Enchant))
                {
                    AscensionMysticEnchant::SetGrantedSpell(player, grant.Enchant, spell, true);
                    managed = true;
                }
            if (!managed)
                player->learnSpell(spell);
            if (!player->HasSpell(spell))
                return false;
            if (save)
                player->SaveToDB(false, false);
            return true;
        }
    return false;
}

std::uint32_t LearnAvailableRanks(Player* player)
{
    std::uint32_t learned = 0;
    for (auto const& rank : RankUpgrades(player))
        learned += LearnRankUpgrade(player, rank.Spell, false) ? 1 : 0;
    if (learned)
        player->SaveToDB(false, false);
    return learned;
}

namespace
{
constexpr char ActiveSlotSetting[] = "core.area52.spec.active";
constexpr char ActiveBuildSetting[] = "core.area52.active_build";
constexpr char PendingBarsKey[] = "area52.specialization.pending_bars";

struct PendingBars : DataMap::Base
{
    std::uint32_t Slot = 0;
    std::map<std::uint8_t, std::uint32_t> Buttons;
    std::chrono::steady_clock::time_point Expires;
};

void RestoreDelayedAction(Player* player, std::uint32_t spell)
{
    auto* pending = player->CustomData.Get<PendingBars>(PendingBarsKey);
    if (!pending || pending->Slot != ActiveSpecialization(player))
        return;
    if (std::chrono::steady_clock::now() >= pending->Expires)
    {
        pending->Buttons.clear();
        return;
    }
    bool changed = false;
    std::erase_if(pending->Buttons, [&](auto const& button)
    {
        if (button.second != spell)
            return false;
        if (!player->GetActionButton(button.first) && player->HasSpell(spell))
        {
            player->addActionButton(button.first, spell, ACTION_BUTTON_SPELL);
            changed = true;
        }
        return true;
    });
    if (changed)
        player->SendActionButtons(1);
}

std::string SlotSetting(std::uint32_t slot, char const* suffix)
{
    return "core.area52.spec." + std::to_string(slot) + suffix;
}

void StoreValues(Player* player, std::string const& key, std::vector<std::uint32_t> const& values)
{
    auto const* old = player->FindPlayerSettings(key);
    std::size_t const count = std::max(values.size(), old ? old->size() : 0);
    for (std::size_t i = 0; i < count; ++i)
        player->UpdatePlayerSetting(key, std::uint32_t(i), i < values.size() ? values[i] : 0);
}

std::vector<std::uint32_t> ReadValues(Player const* player, std::string const& key)
{
    std::vector<std::uint32_t> values;
    if (auto const* saved = player->FindPlayerSettings(key))
        for (auto const& field : *saved)
            values.push_back(field.value);
    return values;
}

void StoreSlot(Player* player, std::uint32_t index)
{
    StoreValues(player, SlotSetting(index, ".entries"), ReadValues(player, SelectionSetting));
    StoreValues(player, SlotSetting(index, ".active_build"), ReadValues(player, ActiveBuildSetting));
    std::vector<std::uint32_t> bars;
    for (std::uint8_t button = 0; button < MAX_ACTION_BUTTONS; ++button)
    {
        auto const* action = player->GetActionButton(button);
        bars.push_back(action ? action->packedData : 0);
    }
    StoreValues(player, SlotSetting(index, ".bars"), bars);
    auto roots = SelectedSpells(KnownEntries(player));
    ExpandGrantedSpells(roots, true);
    std::vector<std::uint32_t> ranks;
    for (auto spell : roots)
        if (player->HasSpell(spell))
            ranks.push_back(spell);
    StoreValues(player, SlotSetting(index, ".ranks"), ranks);
}

class SpecializationSwap final : public SpellScript
{
    PrepareSpellScript(SpecializationSwap);
    std::uint32_t Index = 0;

    bool Load() override
    {
        auto const found = std::find(SpecializationSpells.begin(), SpecializationSpells.end(), GetSpellInfo()->Id);
        if (!Applies(GetCaster()->ToPlayer()) || found == SpecializationSpells.end())
            return false;
        Index = std::uint32_t(found - SpecializationSpells.begin());
        return true;
    }

    SpellCastResult Check()
    {
        Player* player = GetCaster()->ToPlayer();
        if (!player->HasSpell(SpecializationSpells[Index]))
            return SPELL_FAILED_NOT_KNOWN;
        if (player->IsInCombat())
            return SPELL_FAILED_AFFECTING_COMBAT;
        if (player->InBattleground() || player->InArena() || !Area52Build::CanSwitchSpecialization(player))
            return SPELL_FAILED_NOT_HERE;
        return SPELL_CAST_OK;
    }

    void Switch(SpellEffIndex effect)
    {
        PreventHitDefaultEffect(effect);
        std::string error;
        Player* player = GetCaster()->ToPlayer();
        if (!SwitchSpecialization(player, Index, error))
            ChatHandler(player->GetSession()).SendSysMessage(error);
    }

    void Register() override
    {
        OnCheckCast += SpellCheckCastFn(SpecializationSwap::Check);
        OnEffectHitTarget += SpellEffectFn(SpecializationSwap::Switch, EFFECT_0, SPELL_EFFECT_TALENT_SPEC_SELECT);
    }
};

class SpecializationPlayer final : public PlayerScript
{
public:
    SpecializationPlayer() : PlayerScript("Area52SpecializationPlayer",
        { PLAYERHOOK_ON_LOGIN, PLAYERHOOK_ON_LEARN_SPELL }) { }
    void OnPlayerLearnSpell(Player* player, std::uint32_t spell) override
    {
        if (Applies(player))
            RestoreDelayedAction(player, spell);
    }
    void OnPlayerLogin(Player* player) override
    {
        if (Applies(player) && !player->HasSpell(SpecializationSpells[0]))
        {
            std::set<std::uint32_t> const spells{SpecializationSpells[0]};
            SpellLearningScope learning(player, spells);
            player->learnSpell(SpecializationSpells[0]);
        }
    }
};
}

std::uint32_t ActiveSpecialization(Player const* player)
{
    auto const* stored = player->FindPlayerSettings(ActiveSlotSetting);
    auto const slot = stored && !stored->empty() ? (*stored)[0].value : 0;
    return slot < SpecializationSpells.size() ? slot : 0;
}

bool SwitchSpecialization(Player* player, std::uint32_t index, std::string& error)
{
    if (!Applies(player) || index >= SpecializationSpells.size() || !player->HasSpell(SpecializationSpells[index]))
    {
        error = "That specialization is not unlocked.";
        return false;
    }
    if (!player->IsInWorld() || !player->FindMap() || !player->IsAlive() || player->IsInCombat()
        || player->InBattleground() || player->InArena() || player->IsBeingTeleported()
        || player->isMoving() || !Area52Build::CanSwitchSpecialization(player))
    {
        error = "You cannot change specializations right now.";
        return false;
    }
    auto const previous = ActiveSpecialization(player);
    error = AscensionSpecializationSwitchRefusal(player, previous + 1u, index + 1u);
    if (!error.empty())
        return false;
    if (index == previous)
        return true;
    std::vector<KnownEntry> wanted;
    for (auto value : ReadValues(player, SlotSetting(index, ".entries")))
        if (value)
            wanted.push_back({value / 10, value % 10, value % 10, false, std::uint32_t(wanted.size())});
    auto result = Validate(Data, player->GetLevel(), wanted, wanted);
    if (!result)
    {
        error = "The saved specialization is not eligible at your current level or with the current rules.";
        return false;
    }
    StoreSlot(player, previous);
    result = Apply(player, wanted, false, true);
    if (!result)
    {
        error = "The saved specialization could not be restored.";
        return false;
    }
    if (Pet* pet = player->GetPet())
        player->RemovePet(pet, PET_SAVE_NOT_IN_SLOT);
    player->ClearAllReactives();
    player->UnsummonAllTotems();
    player->RemoveAllControlled();
    player->UpdatePlayerSetting(ActiveSlotSetting, 0, index);
    StoreValues(player, ActiveBuildSetting, ReadValues(player, SlotSetting(index, ".active_build")));
    auto ranks = SelectedSpells(wanted);
    ExpandGrantedSpells(ranks, true);
    SpellLearningScope learning(player, ranks);
    for (auto spell : ReadValues(player, SlotSetting(index, ".ranks")))
        if (ranks.contains(spell))
            if (auto const* info = sSpellMgr->GetSpellInfo(spell); info && info->SpellLevel <= player->GetLevel())
                player->learnSpell(spell);
    auto const bars = ReadValues(player, SlotSetting(index, ".bars"));
    auto* pending = player->CustomData.GetDefault<PendingBars>(PendingBarsKey);
    pending->Slot = index;
    pending->Buttons.clear();
    pending->Expires = std::chrono::steady_clock::now() + std::chrono::seconds(10);
    for (std::uint8_t button = 0; button < MAX_ACTION_BUTTONS; ++button)
    {
        player->removeActionButton(button);
        if (button < bars.size() && bars[button])
        {
            player->addActionButton(button, ACTION_BUTTON_ACTION(bars[button]),
                std::uint8_t(ACTION_BUTTON_TYPE(bars[button])));
            if (ACTION_BUTTON_TYPE(bars[button]) == ACTION_BUTTON_SPELL &&
                !player->HasSpell(ACTION_BUTTON_ACTION(bars[button])))
                pending->Buttons[button] = ACTION_BUTTON_ACTION(bars[button]);
        }
    }
    player->SaveToDB(false, false);
    SendArea52Specialization(player);
    Area52Build::RestoreSpecialization(player);
    player->SendActionButtons(1);
    return true;
}

namespace
{
class spell_ascension_ghostly_strike : public SpellScript
{
    PrepareSpellScript(spell_ascension_ghostly_strike);

    bool Load() override { return AscensionFreePick::Enabled() && GetCaster()->IsPlayer(); }

    bool Validate(SpellInfo const*) override
    {
        return ValidateSpellInfo({965819, 865819});
    }

    void Defend(SpellEffIndex)
    {
        Player* player = GetCaster()->ToPlayer();
        if (!player)
            return;
        Item const* weapon = player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_MAINHAND);
        bool const twoHanded = weapon && weapon->GetTemplate()->InventoryType == INVTYPE_2HWEAPON &&
            !player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_OFFHAND);
        player->RemoveAurasDueToSpell(twoHanded ? 965819 : 865819, player->GetGUID());
        player->CastSpell(player, twoHanded ? 865819 : 965819, TRIGGERED_FULL_MASK);
    }

    void Register() override
    {
        OnEffectHitTarget += SpellEffectFn(spell_ascension_ghostly_strike::Defend, EFFECT_1, SPELL_EFFECT_DUMMY);
    }
};

class spell_area52_improved_riposte : public SpellScript
{
    PrepareSpellScript(spell_area52_improved_riposte);

    bool Load() override { return AscensionFreePick::Enabled() && GetCaster()->IsPlayer(); }

    bool Validate(SpellInfo const*) override
    {
        return ValidateSpellInfo({14278, 13713, 13853, 13854, 965835, 965836, 965837});
    }

    void ReduceCooldown()
    {
        Player* player = GetCaster()->ToPlayer();
        if (!player || GetSpell()->IsTriggered())
            return;
        uint32 helper = player->HasAura(13854) ? 965837 :
            player->HasAura(13853) ? 965836 : player->HasAura(13713) ? 965835 : 0;
        if (helper && roll_chance_i(sSpellMgr->AssertSpellInfo(helper)->ProcChance))
            player->ModifySpellCooldown(14278, -3000);
    }

    void Register() override
    {
        AfterCast += SpellCastFn(spell_area52_improved_riposte::ReduceCooldown);
    }
};

}

#include "Area52PrestigeReset.inc"

void RegisterSpecializations()
{
    AscensionPrestigeReset::Area52 = &PrestigeResetHandler;
    RegisterSpellScriptWithArgs(SpecializationSwap, "spell_area52_specialization_swap");
    new SpecializationPlayer();
    RegisterSpellScript(spell_area52_improved_riposte);
    RegisterSpellScript(spell_ascension_ghostly_strike);
}
}
