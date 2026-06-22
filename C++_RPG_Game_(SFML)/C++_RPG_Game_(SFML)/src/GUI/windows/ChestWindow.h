#pragma once
#include "GUI/windows/WindowBase.h"
#include "GUI/widgets/TextWidget.h"
#include "GUI/widgets/ActionSlot.h"
#include "Interfaces/IAbility.h"
#include "worldmap/AssetManager.h"
#include <SFML/Graphics.hpp>
#include <vector>
#include <memory>
#include <functional>

namespace RPG {

    class ChestWindow : public WindowBase {
    public:
        ChestWindow(const ChestWindow&) = delete;
        ChestWindow& operator=(const ChestWindow&) = delete;

        static ChestWindow& getInstance() {
            static ChestWindow instance;
            return instance;
        }

        void showResults(const std::vector<std::shared_ptr<IAbility>>& lootedItems) {
            buildLayout(lootedItems);
            centerOnScreen();
            show();
        }

    private:
        const Spritesheet* m_iconSet = nullptr;
        const sf::Font* m_font = nullptr;

        ChestWindow()
            : WindowBase(
                *AssetManager::getInstance().getFont("PixelFont"), 
                { 300.f, 200.f },
                "Chest Opened!"
            )
        {
            m_font = AssetManager::getInstance().getFont("PixelFont");
            m_iconSet = AssetManager::getInstance().getSpritesheet("AbilityIcons");
        }


        void buildLayout(const std::vector<std::shared_ptr<IAbility>>& lootedItems) {
            m_children.clear();
            float currentY = 400.f;
            float centerX = getSize().x / 2.f;

            if (!m_font || !m_iconSet) return;

            if (!lootedItems.empty()) {
                auto lootLabel = std::make_unique<TextWidget>(
                    *m_font, "Items Found:", 18, sf::Color::White);
                sf::FloatRect lb = lootLabel->getGlobalBounds();
                lootLabel->setPosition({ centerX - lb.size.x / 2.f, currentY });
                addChild(std::move(lootLabel));
                currentY += 30.f;

                const float slotSize = 40.f;
                const float spacing = 10.f;
                int count = static_cast<int>(lootedItems.size());
                float totalWidth = count * slotSize + (count - 1) * spacing;
                float startX = centerX - totalWidth / 2.f;

                for (int i = 0; i < count; ++i) {
                    auto slot = std::make_unique<ActionSlot>(
                        *m_iconSet, *m_font, sf::Vector2f(slotSize, slotSize));
                    slot->setAbility(lootedItems[i]);
                    slot->setPosition({ startX + i * (slotSize + spacing), currentY });
                    addChild(std::move(slot));
                }
                currentY += slotSize + 20.f;
            }
            else {
                auto emptyLabel = std::make_unique<TextWidget>(
                    *m_font, "The chest was empty.", 18, sf::Color(150, 150, 150));
                sf::FloatRect lb = emptyLabel->getGlobalBounds();
                emptyLabel->setPosition({ centerX - lb.size.x / 2.f, currentY });
                addChild(std::move(emptyLabel));
                currentY += 30.f;
            }

            setSize({ getSize().x, currentY + 10.f });
        }

        void centerOnScreen() {
            float windowX = getSize().x;
            float windowY = getSize().y;

            setPosition({ (Window::WIDTH - windowX) / 2.f, (Window::HEIGHT - windowY) / 2.f });
        }

    };
}