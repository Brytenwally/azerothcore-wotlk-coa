/* Copyright (C) 2016+ AzerothCore, GNU AGPL v3. */

#ifndef ASCENSION_SUMMONED_EFFECTS_H
#define ASCENSION_SUMMONED_EFFECTS_H

#include <cstdint>

namespace AscensionSummonedEffects
{
enum class Behaviour : std::uint8_t
{
    Anchor,
    Spawn,
    Pulse,
    Delay,
    Tick,
    Expire,
    Aura,
    PullOwner,
    Freeze,
    Heal,
    Store,
    Attack,
    Turret,
    Mine,
    Vortex,
    Return
};

enum class Motion : std::uint8_t
{
    Stay,
    Forward,
    OutAndBack
};

struct Summon
{
    std::uint32_t SummonSpell;
    std::uint32_t Creature;
    Behaviour Kind;
    std::uint32_t Helper = 0;
    std::uint32_t Payload = 0;
    std::uint32_t Extra = 0;
    std::uint32_t Milliseconds = 0;
    Motion Path = Motion::Stay;
    float Yards = 0.0f;
};

constexpr std::uint32_t LIGHTWELL_CHARGES = 10;
constexpr float LIGHTWELL_RANGE = 20.0f;
constexpr std::uint32_t LIGHTWELL_HEALTH_PCT = 50;
constexpr float MINE_TRIGGER_RADIUS = 3.0f;
constexpr float VORTEX_RADIUS = 5.0f;
constexpr float SLOW_ON_HIT_RATE = 0.1f;

constexpr bool SlowsOnHit(std::uint32_t creature)
{
    return creature == 840047;
}

constexpr Summon Ranked(std::uint32_t summon, std::uint32_t creature, Behaviour kind, std::uint32_t helper,
    std::uint32_t payload, std::uint32_t extra = 0, Motion path = Motion::Stay, float yards = 0.0f)
{
    return { summon, creature, kind, helper, payload, extra, 0, path, yards };
}

constexpr Summon Timed(std::uint32_t summon, std::uint32_t creature, Behaviour kind, std::uint32_t payload,
    std::uint32_t milliseconds, std::uint32_t extra = 0)
{
    return { summon, creature, kind, 0, payload, extra, milliseconds };
}

constexpr Summon Still(std::uint32_t summon, std::uint32_t creature)
{
    return { summon, creature, Behaviour::Anchor };
}

constexpr Summon SUMMONS[] = {
    Timed(81249, 43286, Behaviour::Attack, 0, 0, 81413),
    Timed(281618, 43286, Behaviour::Attack, 0, 0, 81413),
    Timed(1580417, 43286, Behaviour::Attack, 0, 0, 1580397),
    Timed(954514, 105927, Behaviour::Pulse, 954515, 1000, 954568),
    Timed(955062, 105938, Behaviour::Spawn, 954805, 0),

    Timed(954518, 105928, Behaviour::Delay, 954519, 2000),
    Timed(954588, 105931, Behaviour::Delay, 954519, 2000),
    Timed(954589, 105932, Behaviour::Delay, 954519, 2000),
    Timed(954590, 105933, Behaviour::Delay, 954519, 2000),
    Timed(954591, 105934, Behaviour::Delay, 954519, 2000),
    Timed(954592, 105935, Behaviour::Delay, 954519, 2000),
    Timed(954593, 105936, Behaviour::Delay, 954519, 2000),
    Timed(954594, 105937, Behaviour::Delay, 954519, 2000),
    Timed(956046, 903581, Behaviour::Delay, 2304121, 3000),

    Ranked(954802, 105939, Behaviour::Tick, 954847, 954848, 954849),
    Ranked(954909, 105941, Behaviour::Tick, 954915, 954921, 954927),
    Ranked(954910, 105942, Behaviour::Tick, 954916, 954922, 954928),
    Ranked(954911, 105943, Behaviour::Tick, 954917, 954923, 954929),
    Ranked(954912, 105944, Behaviour::Tick, 954918, 954924, 954930),
    Ranked(954913, 105945, Behaviour::Tick, 954919, 954925, 954931),
    Ranked(954914, 105946, Behaviour::Tick, 954920, 954926, 954932),

    Ranked(954861, 840037, Behaviour::Tick, 954834, 954833),
    Ranked(954862, 840037, Behaviour::Tick, 955063, 954868),
    Ranked(954863, 840037, Behaviour::Tick, 955064, 954869),
    Ranked(954864, 840037, Behaviour::Tick, 955065, 954870),
    Ranked(954865, 840037, Behaviour::Tick, 955066, 954871),
    Ranked(954866, 840037, Behaviour::Tick, 955067, 954872),
    Ranked(954867, 840037, Behaviour::Tick, 955068, 954873),

    Ranked(955032, 840040, Behaviour::Tick, 955033, 955034, 955054, Motion::Forward, 35.0f),
    Ranked(955036, 840040, Behaviour::Tick, 955042, 955048, 955054, Motion::Forward, 35.0f),
    Ranked(955037, 840040, Behaviour::Tick, 955043, 955049, 955054, Motion::Forward, 35.0f),
    Ranked(955038, 840040, Behaviour::Tick, 955044, 955050, 955054, Motion::Forward, 35.0f),
    Ranked(955039, 840040, Behaviour::Tick, 955045, 955051, 955054, Motion::Forward, 35.0f),
    Ranked(955040, 840040, Behaviour::Tick, 955046, 955052, 955054, Motion::Forward, 35.0f),
    Ranked(955041, 840040, Behaviour::Tick, 955047, 955053, 955054, Motion::Forward, 35.0f),

    Ranked(760014, 840047, Behaviour::Tick, 760015, 760016, 0, Motion::Forward, 40.0f),
    Ranked(760019, 840047, Behaviour::Tick, 760033, 760026, 0, Motion::Forward, 40.0f),
    Ranked(760020, 840047, Behaviour::Tick, 760034, 760027, 0, Motion::Forward, 40.0f),
    Ranked(760021, 840047, Behaviour::Tick, 760035, 760028, 0, Motion::Forward, 40.0f),
    Ranked(760022, 840047, Behaviour::Tick, 760036, 760029, 0, Motion::Forward, 40.0f),
    Ranked(760023, 840047, Behaviour::Tick, 760037, 760030, 0, Motion::Forward, 40.0f),
    Ranked(760024, 840047, Behaviour::Tick, 760038, 760031, 0, Motion::Forward, 40.0f),
    Ranked(760025, 840047, Behaviour::Tick, 760039, 760032, 0, Motion::Forward, 40.0f),

    Ranked(760060, 840056, Behaviour::Tick, 760061, 760062, 0, Motion::OutAndBack, 30.0f),
    Ranked(760063, 840056, Behaviour::Tick, 760149, 760070, 0, Motion::OutAndBack, 30.0f),
    Ranked(760064, 840056, Behaviour::Tick, 760150, 760071, 0, Motion::OutAndBack, 30.0f),
    Ranked(760065, 840056, Behaviour::Tick, 760151, 760072, 0, Motion::OutAndBack, 30.0f),
    Ranked(760066, 840056, Behaviour::Tick, 760152, 760073, 0, Motion::OutAndBack, 30.0f),
    Ranked(760067, 840056, Behaviour::Tick, 760153, 760074, 0, Motion::OutAndBack, 30.0f),
    Ranked(760068, 840056, Behaviour::Tick, 760154, 760075, 0, Motion::OutAndBack, 30.0f),
    Ranked(760069, 840056, Behaviour::Tick, 760155, 760076, 0, Motion::OutAndBack, 30.0f),

    Ranked(760040, 840048, Behaviour::Expire, 760041, 760042),
    Ranked(760156, 840048, Behaviour::Expire, 760162, 760168),
    Ranked(760157, 840048, Behaviour::Expire, 760163, 760169),
    Ranked(760158, 840048, Behaviour::Expire, 760164, 760170),
    Ranked(760159, 840048, Behaviour::Expire, 760165, 760171),
    Ranked(760160, 840048, Behaviour::Expire, 760166, 760172),
    Ranked(760161, 840048, Behaviour::Expire, 760167, 760173),

    Ranked(954854, 840036, Behaviour::Freeze, 954857, 954855, 954856),
    Ranked(760053, 840058, Behaviour::Aura, 760054, 0),
    Timed(760056, 840057, Behaviour::PullOwner, 760057, 0),

    Ranked(954504, 840034, Behaviour::Vortex, 954505, 954506),
    Ranked(86401, 80221, Behaviour::Return, 86401, 0),

    Still(760080, 840059),

    Ranked(760009, 841000, Behaviour::Store, 760010, 760011, 30),

    Timed(954611, 855350, Behaviour::Attack, 413110, 2000),

};

constexpr Summon const* Find(std::uint32_t summonSpell, std::uint32_t creature)
{
    for (Summon const& summon : SUMMONS)
        if (summon.SummonSpell == summonSpell && summon.Creature == creature)
            return &summon;
    return nullptr;
}

constexpr Summon const* FindByHelper(std::uint32_t helper, Behaviour kind)
{
    for (Summon const& summon : SUMMONS)
        if (summon.Helper == helper && summon.Kind == kind)
            return &summon;
    return nullptr;
}

constexpr bool UsesHelper(Behaviour kind)
{
    return kind == Behaviour::Tick || kind == Behaviour::Expire || kind == Behaviour::Aura ||
        kind == Behaviour::Freeze;
}
}

#endif
