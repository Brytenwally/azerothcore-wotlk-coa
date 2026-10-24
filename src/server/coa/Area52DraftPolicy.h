#ifndef AREA52_DRAFT_POLICY_H
#define AREA52_DRAFT_POLICY_H

#include <cstdint>

namespace Area52Build
{
struct DraftProgress
{
    std::uint64_t Build = 0;
    bool Forfeited = false;
    bool Rewarded = false;
};

inline DraftProgress SwitchDraft(DraftProgress before, std::uint32_t level,
    std::uint64_t nextBuild, bool featured)
{
    std::uint64_t const next = featured ? nextBuild : 0;
    if (level >= 10 && next != before.Build)
        before.Forfeited = true;
    before.Build = next;
    return before;
}

inline bool CanCompleteDraft(DraftProgress const& state, std::uint32_t level, std::uint32_t maximumLevel)
{
    return state.Build && !state.Forfeited && !state.Rewarded && maximumLevel >= 10 && level >= maximumLevel;
}
}

#endif
