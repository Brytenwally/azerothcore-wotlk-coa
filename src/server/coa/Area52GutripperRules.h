#ifndef AREA52_GUTRIPPER_RULES_H
#define AREA52_GUTRIPPER_RULES_H
#include <algorithm>
#include <cstdint>

namespace Area52Gutripper
{
constexpr std::uint32_t BloodAndGuts = 977906;
constexpr std::uint32_t Disembowel = 947470;
constexpr std::uint32_t Eviscerate = 2098;
constexpr std::uint32_t ExecutingDisembowelAura = 977908;
constexpr std::uint32_t ExecutingDisembowel = 977909;
constexpr std::uint32_t RemorselessAttacks = 14144;
constexpr std::uint32_t RemorselessAttacksCapstone = 14148;
constexpr std::uint32_t RemorselessBuff = 14143;
constexpr std::uint32_t RemorselessCapstoneBuff = 14149;
constexpr std::uint32_t DeadlyCombo = 977856;
constexpr std::uint32_t DeadlyComboBuff = 954353;
constexpr std::uint32_t SuddenDeathProc = 52437;
constexpr std::uint32_t SuddenDeathMaxRank = 29724;
constexpr std::uint32_t SuddenDeathCooldown = 411167;
constexpr std::uint32_t PiercingCleaverEnchant = 982965;
constexpr std::uint32_t PiercingCleaver = 982966;
constexpr std::uint32_t PiercingHowl = 12323;
constexpr std::uint32_t Weakness = 982967;
constexpr std::uint32_t JudgeJuryExecutioner = 84447;
constexpr std::uint32_t JudgeJuryExecutionerBuff = 84448;
constexpr std::uint32_t ExecuteDamage = 20647;
constexpr std::uint32_t DeathSentence = 982106;
constexpr std::uint32_t BeStillAndDie = 982101;
constexpr std::uint32_t JudgementOfJustice = 20184;
constexpr std::uint32_t HammerOfJustice = 853;
constexpr std::uint32_t NecroticExecute = 782820;
constexpr std::uint32_t NecroticExecuteHelper = 782821;
constexpr std::uint32_t NecroticPower = 782800;
constexpr std::uint32_t WorldforgedCooldown = 780999;
constexpr std::int32_t DeadlyComboChancePerPoint = 20;
constexpr float DisembowelPowerCoefficient = 0.0176f;

inline std::int32_t DisembowelPowerBonus(std::uint8_t comboPoints, float attackPlusSpellPower)
{
    return std::int32_t(DisembowelPowerCoefficient * attackPlusSpellPower * comboPoints * comboPoints);
}

inline std::int32_t DeadlyComboChance(std::uint8_t comboPoints)
{
    return std::min<std::int32_t>(100, DeadlyComboChancePerPoint * comboPoints);
}

inline std::int32_t WithExecuteBonus(std::int32_t damage, std::int32_t bonusPct)
{
    return bonusPct > 0 ? std::int32_t(std::int64_t(damage) * (100 + bonusPct) / 100) : damage;
}
}
#endif
