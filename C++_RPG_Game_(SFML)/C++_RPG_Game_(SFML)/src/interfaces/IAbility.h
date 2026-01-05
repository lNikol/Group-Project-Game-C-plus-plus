#pragma once
#include <string>

namespace RPG {

    class IAbility {
    public:
        virtual ~IAbility() = default;

        // Visuals
        virtual int getIconIndex() const = 0;
        virtual std::string getTooltip() const = 0;

        // Mechanics
        virtual bool canBeCast() const = 0; // Checks cost
        virtual int getCharges() const = 0; // -1 if infinite/not applicable
        virtual int getCooldown() const = 0; // 0 if ready

        // Execution
        virtual void execute() = 0;
    };
}