#pragma once
#include "CombatWorldObject.h"

namespace RPG {

    /**
     * @brief Represents static environmental objects.
     * * Props can be blocking (walls) or decorative (rugs).
     */
    class Prop : public CombatWorldObject {
    public:
        Prop(bool blocksMove, bool blocksSight, std::string name = "Prop")
            : m_blocksMove(blocksMove), m_blocksSight(blocksSight), m_name(name) {
        }

        virtual ~Prop() = default;

        // ==============================
        // Overrides from CombatWorldObject
        // ==============================

        bool blocksMovement() const override { return m_blocksMove; }
        bool blocksSight() const override { return m_blocksSight; }
        std::string getName() const override { return m_name; }

    private:
        bool m_blocksMove;
        bool m_blocksSight;
        std::string m_name;
    };
}