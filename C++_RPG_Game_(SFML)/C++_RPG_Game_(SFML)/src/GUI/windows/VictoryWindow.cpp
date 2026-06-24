#include "VictoryWindow.h"

namespace RPG {
    VictoryWindow::VictoryWindow(const Spritesheet& iconSet, const sf::Font& font, std::function<void()> onCloseCallback)
        : WindowBase(font, sf::Vector2f(300.f, 250.f), "Victory!"),
        m_iconSet(iconSet),
        m_font(font),
        m_onCloseCallback(onCloseCallback)
    {
    }

    bool VictoryWindow::handleEvent(sf::RenderWindow& window, const sf::Event& event) {
        // Run standard window logic (dragging, children clicks, closing)
        bool consumed = WindowBase::handleEvent(window, event);

        // If the window is no longer visible after handleEvent, the Close button was clicked
        if (!m_isVisible && m_onCloseCallback) {
            m_onCloseCallback();
            m_onCloseCallback = nullptr; // Ensure it only fires once
        }

        return consumed;
    }

    void VictoryWindow::setResults(int goldReward, const std::vector<std::shared_ptr<IAbility>>& lootedItems) {
        buildLayout(goldReward, lootedItems);
        show();
    }

    void VictoryWindow::buildLayout(int goldReward, const std::vector<std::shared_ptr<IAbility>>& lootedItems) {
        m_children.clear(); // Wipe any previous layouts

        float currentY = 40.f;
        float centerX = getSize().x / 2.f;

        // 1. Add Gold Display
        std::string goldStr = "+ " + std::to_string(goldReward) + " Gold";
        auto goldText = std::make_unique<TextWidget>(m_font, goldStr, 20, sf::Color::Yellow);

        sf::FloatRect textBounds = goldText->getGlobalBounds();
        goldText->setPosition({ centerX - (textBounds.size.x / 2.f), currentY });
        addChild(std::move(goldText));

        currentY += 40.f;

        // 2. Add Loot Display
        if (!lootedItems.empty()) {
            auto lootLabel = std::make_unique<TextWidget>(m_font, "Loot Acquired:", 16, sf::Color::White);
            sf::FloatRect labelBounds = lootLabel->getGlobalBounds();
            lootLabel->setPosition({ centerX - (labelBounds.size.x / 2.f), currentY });
            addChild(std::move(lootLabel));

            currentY += 30.f;

            // Generate Action Slots dynamically
            float slotSize = 40.f;
            float spacing = 10.f;
            int itemsCount = lootedItems.size();

            // Calculate starting X to keep the row perfectly centered
            float totalWidth = (itemsCount * slotSize) + ((itemsCount - 1) * spacing);
            float startX = centerX - (totalWidth / 2.f);

            for (int i = 0; i < itemsCount; ++i) {
                auto slot = std::make_unique<ActionSlot>(m_iconSet, m_font, sf::Vector2f(slotSize, slotSize));
                slot->setAbility(lootedItems[i]);
                slot->setPosition({ startX + i * (slotSize + spacing), currentY });
                addChild(std::move(slot));
            }
            currentY += slotSize + 20.f;
        }
        else {
            // Edge case: No loot dropped
            auto noLootLabel = std::make_unique<TextWidget>(m_font, "No items dropped.", 16, sf::Color(150, 150, 150));
            sf::FloatRect labelBounds = noLootLabel->getGlobalBounds();
            noLootLabel->setPosition({ centerX - (labelBounds.size.x / 2.f), currentY });
            addChild(std::move(noLootLabel));
            currentY += 30.f;
        }

        // Dynamically shrink/grow window based on total height
        setSize({ getSize().x, currentY + 10.f });
    }
}