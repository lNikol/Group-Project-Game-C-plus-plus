#include "InventoryWindow.h"

namespace RPG {
    InventoryWindow::InventoryWindow(const Spritesheet& iconSet, const sf::Font& font)
        : WindowBase(font, sf::Vector2f(250.f, 220.f), "Inventory") {
        buildLayout(iconSet, font);
    }

    void InventoryWindow::buildLayout(const Spritesheet& iconSet, const sf::Font& font) {
        float startX = 10.f, startY = 40.f, size = 40.f, spacing = 5.f;
        for (int i = 0; i < INVENTORY_SIZE; ++i) {
            auto slot = std::make_unique<ActionSlot>(iconSet, font, sf::Vector2f(size, size));
            slot->setPosition({ startX + (i % 5) * (size + spacing), startY + (i / 5) * (size + spacing) });
            m_bagSlots.push_back(slot.get());
            addChild(std::move(slot));
        }
    }

    void InventoryWindow::update(float dt) {
        WindowBase::update(dt);
        if (!m_isVisible) return;

        PartyData::getInstance().clearConsumedItems();
        auto& inv = PartyData::getInstance().sharedInventory;
        for (size_t i = 0; i < m_bagSlots.size(); ++i) {
            if (m_bagSlots[i]->getAbility() != inv[i]) m_bagSlots[i]->setAbility(inv[i]);
            m_bagSlots[i]->update(dt);
        }
    }
}