/* Copyright (C) 2016+ AzerothCore, GNU AGPL v3. */
#ifndef ASCENSION_MYSTIC_AUDIT_H
#define ASCENSION_MYSTIC_AUDIT_H

#include "AscensionMysticSpellGrants.h"
#include "AscensionFreePickLinkedSpells.h"
#include "Area52CompanionTraining.h"
#include <boost/bind/placeholders.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ptree.hpp>
#include <filesystem>

namespace AscensionMysticEnchant
{
inline bool ExportAudit(Catalog const& catalog, std::map<std::uint32_t, std::uint32_t> const& connected,
    std::string const& path)
{
    using boost::property_tree::ptree;
    ptree document, entries, advancement, spells;
    std::vector<uint32> pending;
    std::set<uint32> visited;
    auto const clientChains = std::filesystem::path(path).parent_path() / "client-chains.json";
    if (std::filesystem::is_regular_file(clientChains))
    {
        ptree chains;
        boost::property_tree::read_json(clientChains.string(), chains);
        for (auto const& chain : chains)
            for (auto const& rank : chain.second)
                pending.push_back(rank.second.get_value<uint32>());
    }
    auto const seedPath = std::filesystem::path(path).parent_path() / "audit-seeds.json";
    if (std::filesystem::is_regular_file(seedPath))
    {
        ptree seeds;
        boost::property_tree::read_json(seedPath.string(), seeds);
        for (auto const& seed : seeds)
            pending.push_back(seed.second.get_value<uint32>());
    }
    for (auto const& [id, entry] : catalog.BySpell)
    {
        ptree row;
        row.put("spell", id);
        row.put("catalog", entry.Id);
        row.put("quality", entry.Quality);
        row.put("connected", connected.contains(id));
        entries.push_back({"", row});
        pending.push_back(id);
    }
    AscensionFreePick::Catalog abilities;
    bool const abilitiesLoaded = AscensionFreePick::LoadCatalog(abilities);
    for (auto const& [id, entry] : abilities.Entries)
    {
        ptree row, ranks, connectedEntries;
        row.put("entry", id);
        row.put("ae", entry.AE);
        row.put("te", entry.TE);
        row.put("level", entry.Level);
        row.put("flags", entry.Flags);
        for (uint32 spell : entry.Spells)
        {
            ptree value;
            value.put("", spell);
            ranks.push_back({"", value});
            pending.push_back(spell);
        }
        for (uint32 prerequisite : entry.Connected)
        {
            ptree value;
            value.put("", prerequisite);
            connectedEntries.push_back({"", value});
        }
        row.add_child("spells", ranks);
        row.add_child("connected_entries", connectedEntries);
        advancement.push_back({"", row});
    }
    std::map<uint32, std::vector<uint32>> scriptedGrants = {
        {81116, {983702, 983703, 983704, 983705, 983706, 983707}},
        {92580, {92583}},
        {926573, {926574, 926576, 926578, 926580, 926582, 926584, 926586, 926588}},
        {954081, {954082, 954443, 954444, 954445, 954446, 954447, 954448, 954449, 954450}},
        {978217, {978533, 978569, 982250, 901200}}
    };
    for (auto const& grant : SpellGrants)
        scriptedGrants[grant.Enchant].push_back(grant.Spell);
    for (auto const& group : AscensionFreePick::LinkedSpellGroups)
        for (uint32 child : group.Children)
            if (child)
                scriptedGrants[group.Parent].push_back(child);
    for (auto spell : Area52CompanionTraining::Spells)
        scriptedGrants[Area52CompanionTraining::Mastery].push_back(spell);
    for (auto spell : Area52CompanionTraining::Runes)
        scriptedGrants[Area52CompanionTraining::Runeforging].push_back(spell);
    while (!pending.empty())
    {
        uint32 const id = pending.back();
        pending.pop_back();
        if (!visited.insert(id).second)
            continue;
        ptree row, effects, edges, scripts;
        row.put("id", id);
        SpellInfo const* info = sSpellMgr->GetSpellInfo(id);
        row.put("present", info != nullptr);
        if (!info)
        {
            spells.push_back({"", row});
            continue;
        }
        row.put("name", info->SpellName[0]);
        row.put("level", info->SpellLevel);
        row.put("power_type", int32(info->PowerType));
        row.put("flat_cost", info->ManaCost);
        row.put("percent_cost", info->ManaCostPercentage);
        row.put("family", info->SpellFamilyName);
        row.put("school", info->SchoolMask);
        row.put("duration_ms", info->GetDuration());
        row.put("cast_ms", info->CastTimeEntry ? info->CastTimeEntry->CastTime : 0);
        row.put("channel", info->IsChanneled());
        row.put("interrupt", info->InterruptFlags);
        row.put("channel_interrupt", info->ChannelInterruptFlags);
        row.put("stack_amount", info->StackAmount);
        row.put("ignores_caster_modifiers", !info->IsAffectedBySpellMods());
        row.put("attributes0", info->Attributes);
        row.put("attributes1", info->AttributesEx);
        row.put("attributes2", info->AttributesEx2);
        row.put("attributes3", info->AttributesEx3);
        row.put("attributes4", info->AttributesEx4);
        row.put("attributes5", info->AttributesEx5);
        row.put("attributes6", info->AttributesEx6);
        row.put("attributes7", info->AttributesEx7);
        if (auto const* bonus = sSpellMgr->GetSpellBonusData(id))
        {
            ptree values;
            values.put("direct", bonus->direct_damage);
            values.put("periodic", bonus->dot_damage);
            values.put("attack_power_direct", bonus->ap_bonus);
            values.put("attack_power_periodic", bonus->ap_dot_bonus);
            row.add_child("coefficient_override", values);
        }
        for (uint32 i = 0; i < 3; ++i)
            row.put("family_mask" + std::to_string(i), info->SpellFamilyFlags[i]);
        row.put("passive", info->IsPassive());
        row.put("rank_root", sSpellMgr->GetFirstSpellInChain(id));
        row.put("rank", uint32(info->GetRank()));
        row.put("proc_entry", sSpellMgr->GetSpellProcEntry(id) != nullptr);
        row.put("proc_flags", info->ProcFlags);
        row.put("proc_chance", info->ProcChance);
        if (auto const* proc = sSpellMgr->GetSpellProcEntry(id))
        {
            ptree values;
            values.put("SchoolMask", proc->SchoolMask);
            values.put("SpellFamilyName", proc->SpellFamilyName);
            for (uint32 i = 0; i < 3; ++i)
                values.put("SpellFamilyMask" + std::to_string(i), proc->SpellFamilyMask[i]);
            values.put("ProcFlags", proc->ProcFlags);
            values.put("SpellTypeMask", proc->SpellTypeMask);
            values.put("SpellPhaseMask", proc->SpellPhaseMask);
            values.put("HitMask", proc->HitMask);
            values.put("AttributesMask", proc->AttributesMask);
            values.put("DisableEffectsMask", proc->DisableEffectsMask);
            values.put("ProcsPerMinute", proc->ProcsPerMinute);
            values.put("Chance", proc->Chance);
            values.put("Cooldown", proc->Cooldown.count());
            values.put("Charges", proc->Charges);
            row.add_child("proc", values);
        }
        auto edge = [&](uint32 target, char const* kind)
        {
            if (!target)
                return;
            ptree value;
            value.put("target", target);
            value.put("kind", kind);
            edges.push_back({"", value});
            pending.push_back(target);
        };
        edge(sSpellMgr->GetNextSpellInChain(id), "next_rank");
        for (auto const& effect : info->Effects)
        {
            if (!effect.IsEffect())
                continue;
            ptree value;
            value.put("index", uint32(effect.EffectIndex));
            value.put("effect", effect.Effect);
            value.put("aura", effect.ApplyAuraName);
            value.put("misc", effect.MiscValue);
            value.put("trigger", effect.TriggerSpell);
            value.put("base_points", effect.BasePoints);
            value.put("die_sides", effect.DieSides);
            value.put("amplitude_ms", effect.Amplitude);
            value.put("target_a", effect.TargetA.GetTarget());
            value.put("target_b", effect.TargetB.GetTarget());
            value.put("bonus_multiplier", effect.BonusMultiplier);
            value.put("points_per_level", effect.RealPointsPerLevel);
            for (uint32 i = 0; i < 3; ++i)
                value.put("class_mask" + std::to_string(i), effect.SpellClassMask[i]);
            value.put("effect_handler", effect.Effect < TOTAL_SPELL_EFFECTS &&
                SpellEffects[effect.Effect] != &Spell::EffectNULL &&
                SpellEffects[effect.Effect] != &Spell::EffectUnused);
            value.put("aura_handler", !effect.ApplyAuraName || (effect.ApplyAuraName < TOTAL_AURAS &&
                AuraEffectHandler[effect.ApplyAuraName] != &AuraEffect::HandleNULL &&
                AuraEffectHandler[effect.ApplyAuraName] != &AuraEffect::HandleUnused));
            effects.push_back({"", value});
            edge(effect.TriggerSpell, effect.Effect == SPELL_EFFECT_LEARN_SPELL ? "learn" : "trigger");
            if (effect.ApplyAuraName == SPELL_AURA_OVERRIDE_SPELLS)
                if (auto const* overrides = sOverrideSpellDataStore.LookupEntry(uint32(effect.MiscValue)))
                    for (uint32 spell : overrides->spellId)
                        edge(spell, "override_grant");
        }
        if (auto const found = scriptedGrants.find(id); found != scriptedGrants.end())
            for (uint32 spell : found->second)
                edge(spell, "reviewed_script_grant");
        for (int32 type = 0; type < 3; ++type)
            if (auto const* links = sSpellMgr->GetSpellLinked(int32(id) + type * SPELL_LINKED_MAX_SPELLS))
                for (int32 target : *links)
                    if (target > 0)
                        edge(uint32(target), "linked");
        std::list<SpellScript*> spellScripts;
        std::list<AuraScript*> auraScripts;
        sScriptMgr->CreateSpellScripts(id, spellScripts);
        sScriptMgr->CreateAuraScripts(id, auraScripts);
        auto names = [&](auto& values)
        {
            for (auto* script : values)
            {
                ptree name;
                name.put("", *script->_GetScriptName());
                scripts.push_back({"", name});
                delete script;
            }
        };
        names(spellScripts);
        names(auraScripts);
        row.add_child("effects", effects);
        row.add_child("edges", edges);
        row.add_child("scripts", scripts);
        spells.push_back({"", row});
    }
    document.put("schema", 1);
    document.put("advancement_loaded", abilitiesLoaded);
    document.put("scope", "effective structural connectivity; not gameplay certification");
    document.add_child("catalog", entries);
    document.add_child("advancement", advancement);
    document.add_child("spells", spells);
    try
    {
        boost::property_tree::write_json(path, document);
        return true;
    }
    catch (std::exception const&)
    {
        return false;
    }
}
}
#endif
