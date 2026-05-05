#pragma once
#include "GUI/windows/WindowBase.h"
#include "GUI/widgets/ActionSlot.h"
#include "combat/PartyData.h"

namespace RPG {
    class InventoryWindow : public WindowBase {
    public:
        InventoryWindow(const Spritesheet& iconSet, const sf::Font& font);
        void update(float dt) override;

    private:
        std::vector<ActionSlot*> m_bagSlots;
        void buildLayout(const Spritesheet& iconSet, const sf::Font& font);
    };
}