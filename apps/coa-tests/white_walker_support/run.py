import argparse
from pathlib import Path
import shutil
import subprocess
import tempfile

ROOT = Path(__file__).resolve().parents[3]

MAIN = r'''
#include "AscensionWhiteWalkerSupportRules.h"
#include <cassert>
#include <algorithm>
#include <vector>
#include <map>
using namespace AscensionWhiteWalkerSupport;
int main()
{
    static_assert(AthleticCostPercent == -4 && AthleticSchools == 17);
    static_assert(WindsOfWinterRanks.size() == 8 && WindsOfWinterRanks.front() == 978846 && WindsOfWinterRanks.back() == 978853);
    static_assert(IceSpearRanks.size() == 7 && IceSpearRanks.front() == 984935 && IceSpearRanks.back() == 984975);
    static_assert(WintersFury == 984936 && WintersVitality == 978525);
    static_assert(FrostbrandDamageHelpers.size() == 9 && FrostbrandDamageHelpers.front() == 965990 && FrostbrandDamageHelpers.back() == 965998);
    static_assert(FrostbrandSlowHelpers.size() == 9 && FrostbrandSlowHelpers.front() == 8034 && FrostbrandSlowHelpers.back() == 58799);
    static_assert(FrozenPowerTalentPercent(0) == 3 && FrozenPowerTalentPercent(1) == 6 && FrozenPowerTalentPercent(2) == 9);
    static_assert(FrozenPowerMysticPercent(1) == 2 && FrozenPowerMysticPercent(2) == 4 && FrozenPowerMysticPercent(3) == 6 && FrozenPowerMysticPercent(4) == 6);
    static_assert(FrozenPowerSchoolAllowed(4) && FrozenPowerSchoolAllowed(8) && FrozenPowerSchoolAllowed(16));
    static_assert(!FrozenPowerSchoolAllowed(1) && !FrozenPowerSchoolAllowed(32));
    static_assert(EviscerateClientCoefficient(1) == 2 && EviscerateClientCoefficient(5) == 50);
    assert(HonedIceSource(3, 512, 16, false, 1, true));
    assert(!HonedIceSource(3, 512, 16, true, 1, true));
    assert(!HonedIceSource(3, 512, 16, false, 0, true));
    assert(!HonedIceSource(3, 512, 16, false, 1, false));
    assert(!HonedIceSource(3, 1, 16, false, 1, true));
    assert(!HonedIceSource(3, 512, 4, false, 1, true));
    assert(!HonedIceSource(8, 512, 16, false, 1, true));
    assert(HonedIceCriticalBonus(2, 1) == 2);
    assert(HonedIceCriticalBonus(2, 3) == 6);
    assert(HonedIceCriticalBonus(2, 0) == 0);
    assert(WhiteWalkerActive(true, true, true));
    assert(!WhiteWalkerActive(false, true, true));
    assert(!WhiteWalkerActive(true, false, true));
    assert(!WhiteWalkerActive(true, true, false));
}
'''

FROSTBRAND_MOCKS = r'''
using uint32 = std::uint32_t;
struct Unit {};
struct ObjectGuid { int value = 0; bool operator==(ObjectGuid const& other) const { return value == other.value; } };
struct Item { ObjectGuid GetGUID() const { return {}; } };
struct SpellInfo { uint32 Id; };
constexpr int TRIGGERED_FULL_MASK = 1;
bool Area52Enabled = true;
struct Aura { ObjectGuid GetCastItemGUID() const { return {}; } };
struct AuraApplication { Aura* GetBase() const { static Aura aura; return &aura; } };
struct AllItemScript
{
    explicit AllItemScript(char const*) {}
    virtual bool OnItemCombatSpell(struct Player*, Unit*, SpellInfo const*, Item*) { return true; }
};
struct Player
{
    struct Cast { Unit* target; uint32 spell; Item* item; };
    std::vector<Cast> casts;
    std::multimap<uint32, AuraApplication*> auras;
    auto const& GetAppliedAuras() const { return auras; }
    void CastSpell(Unit* target, uint32 spell, int, Item* item) { casts.push_back({target, spell, item}); }
};
'''

FROSTBRAND_CASES = r'''
    item_area52_frostbrand handler;
    Player player;
    Unit victim;
    Item weapon;
    constexpr std::array<uint32, 9> helpers = {965990,965991,965992,965993,965994,965995,965996,965997,965998};
    constexpr std::array<uint32, 9> attacks = {8034,8037,10458,16352,16353,965999,58797,58798,58799};
    for (std::size_t rank = 0; rank < helpers.size(); ++rank)
    {
        SpellInfo spell{helpers[rank]};
        assert(handler.OnItemCombatSpell(&player, &victim, &spell, &weapon));
        assert(player.casts.size() == rank + 1);
        assert(player.casts.back().target == &victim);
        assert(player.casts.back().spell == attacks[rank]);
        assert(player.casts.back().item == &weapon);
    }
    for (uint32 id : {0u, 965989u, 965999u, 58799u})
    {
        SpellInfo unrelated{id};
        assert(handler.OnItemCombatSpell(&player, &victim, &unrelated, &weapon));
    }
    SpellInfo helper{965990};
    assert(handler.OnItemCombatSpell(nullptr, &victim, &helper, &weapon));
    assert(handler.OnItemCombatSpell(&player, nullptr, &helper, &weapon));
    assert(handler.OnItemCombatSpell(&player, &victim, nullptr, &weapon));
    Area52Enabled = false;
    assert(handler.OnItemCombatSpell(&player, &victim, &helper, &weapon));
    assert(player.casts.size() == 9);
'''

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--dbc-dir')
    parser.parse_args()
    source = (ROOT / 'src/server/coa/AscensionWhiteWalkerSupport.cpp').read_text()
    assert 'OnPlayerCanApplyEnchantment' in source
    assert 'FrostbrandEnchantments' in source
    assert 'RemoveAurasDueToItemSpell(helper, item->GetGUID())' in source
    assert 'RemoveAurasDueToItemSpell(otherHelper, item->GetGUID())' in source
    assert 'GetCastItemGUID() == itemGuid' in source
    assert 'HasItemHelper(player, helper, item->GetGUID())' in source
    assert 'AbsoluteZeroReplacement' in source
    assert 'SetTemporarySpellReplacement(FrostNova, AbsoluteZeroReplacement)' in source
    assert 'SetTemporarySpellReplacement(FrostNova, 0)' in source
    assert 'AbsoluteZeroGranted.erase(guid)' in source
    assert 'event.GetSpellInfo()->Id == AscensionWhiteWalkerSupport::ColdBlood' in source
    assert 'player->CastSpell(player, AscensionWhiteWalkerSupport::Frostbolt' in source
    assert 'getMSTimeDiff(last, now) < 1000' in source
    assert 'ModifySpellCooldown(AscensionWhiteWalkerSupport::ColdBlood, -10000)' in source
    conditional = (ROOT / 'src/server/coa/AscensionConditionalCombat.cpp').read_text()
    assert 'ApplyFrozenPowerDamageContract' in conditional
    assert 'SPELL_AURA_MOD_DAMAGE_DONE_VERSUS_AURASTATE' in conditional
    assert 'ASCENSION_CLASSMASK_FROSTBRAND_DAMAGE' in conditional
    assert 'FrozenPowerTalentPercent' in conditional and 'FrozenPowerMysticPercent' in conditional
    assert '8034, 8037, 10458, 16352, 16353, 965999, 58797, 58798, 58799' in (ROOT / 'src/server/game/Entities/Unit/Unit.cpp').read_text()
    handler = source.split('class item_area52_frostbrand :', 1)[1].split('class player_area52_frostbrand', 1)[0]
    handler = 'class item_area52_frostbrand :' + handler
    generated = MAIN.replace('int main()', FROSTBRAND_MOCKS + handler + '\nint main()')
    generated = generated.replace('    static_assert(AthleticCostPercent', FROSTBRAND_CASES + '\n    static_assert(AthleticCostPercent', 1)
    player_source = (ROOT / 'src/server/game/Entities/Player/Player.cpp').read_text()
    enchant_path = player_source.split('// item combat enchantments', 1)[1].split('void Player::CastItemUseSpell', 1)[0]
    assert 'OnCastItemCombatSpell(this, target, spellInfo, item)' in enchant_path
    dispatcher = (ROOT / 'src/server/game/Scripting/ScriptDefines/AllItemScript.cpp').read_text()
    assert 'script->OnItemCombatSpell(player, victim, spellInfo, item)' in dispatcher
    compiler = shutil.which('c++') or shutil.which('g++')
    if not compiler:
        raise SystemExit('UNAVAILABLE: C++ compiler required')
    with tempfile.TemporaryDirectory(prefix='white-walker-support-') as temporary:
        folder = Path(temporary)
        source = folder / 'rules.cpp'
        binary = folder / 'rules'
        source.write_text(generated)
        subprocess.run([compiler, '-std=c++20', '-Wall', '-Wextra', '-Werror',
                        '-I', str(ROOT / 'src/server/coa'), str(source), '-o', str(binary)], check=True)
        subprocess.run([str(binary)], check=True)
    print('PASS: White Walker support rules; production Frostbrand handler all ranks, victim/item ownership and exclusions')

if __name__ == '__main__':
    main()
