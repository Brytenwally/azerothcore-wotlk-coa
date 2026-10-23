#include "Area52CastMovementRules.h"

class area52_cast_movement_metadata : public GlobalScript
{
public:
    area52_cast_movement_metadata() : GlobalScript("area52_cast_movement_metadata") { }

    void OnLoadSpellCustomAttr(SpellInfo* info) override
    {
        if (!Area52CastMovement::RequiresInterruption(info->Id))
            return;
        if (!sConfigMgr->GetOption<bool>("CoA.Enable", true) ||
            sConfigMgr->GetOption<std::string>("CoA.ClassModel", "coa") != "hero" ||
            sConfigMgr->GetOption<std::string>("CoA.RealmType", "live") != "live" ||
            (sConfigMgr->GetOption<uint32>("CoA.GameModeMask", 0) & ~uint32(0x400)))
            return;
        info->InterruptFlags |= SPELL_INTERRUPT_FLAG_MOVEMENT;
    }
};
