#pragma once
#include "GUI/windows/WindowBase.h"
#include "GUI/widgets/ActionSlot.h"
#include "GUI/widgets/TextWidget.h"
#include "Interfaces/IAbility.h"
#include "worldmap/AssetManager.h"
#include "core/Constants.h"
#include <vector>
#include <memory>
#include <functional>

namespace RPG {

    /**
     * @brief Modal window shown when the player opens a chest.
     * Mirrors VictoryWindow layout but without gold — shows items only.
     * Singleton: access via ChestWindow::getInstance().
     */
    class ChestWindow : public WindowBase {
    public:
        static ChestWindow& getInstance() {
            static ChestWindow instance;
            return instance;
        }

        /**
         * @brief Populates the window with looted items, centers it, and shows it.
         * Pass addedItems (only what actually fit into inventory).
         */
        void setResults(const std::vector<std::shared_ptr<IAbility>>& lootedItems) {
            buildLayout(lootedItems);
            sf::Vector2f size = getSize();
            setPosition({
                (Window::WIDTH - size.x) / 2.f,
                (Window::HEIGHT - size.y) / 2.f
                });

            show();
        }

        bool handleEvent(sf::RenderWindow& window, const sf::Event& event) override {
            return WindowBase::handleEvent(window, event);
            return true;
        }

    private:
        const Spritesheet* m_iconSet = nullptr;
        const sf::Font* m_font = nullptr;

        ChestWindow()
            : WindowBase(
                *AssetManager::getInstance().getFont("PixelFont"),
                sf::Vector2f(300.f, 250.f),
                "Chest Opened!"
            )
        {
            m_font = AssetManager::getInstance().getFont("PixelFont");
            m_iconSet = AssetManager::getInstance().getSpritesheet("AbilityIcons");
        }

        void buildLayout(const std::vector<std::shared_ptr<IAbility>>& lootedItems) {
            m_children.clear();

            if (!m_font || !m_iconSet) return;

            float currentY = 40.f;
            float centerX = getSize().x / 2.f;

            if (!lootedItems.empty()) {
                // Header
                auto header = std::make_unique<TextWidget>(
                    *m_font, "You got loot!", 24, sf::Color::Black);
                sf::FloatRect hb = header->getGlobalBounds();
                header->setPosition({ centerX - hb.size.x / 2.f, currentY });
                addChild(std::move(header));
                currentY += 40.f;

                // Item icon row — centered, same as VictoryWindow
                const float slotSize = 40.f;
                const float spacing = 10.f;
                int   count = static_cast<int>(lootedItems.size());
                float totalWidth = count * slotSize + (count - 1) * spacing;
                float startX = centerX - totalWidth / 2.f;

                for (int i = 0; i < count; ++i) {
                    auto slot = std::make_unique<ActionSlot>(
                        *m_iconSet, *m_font, sf::Vector2f(slotSize, slotSize));
                    slot->setAbility(lootedItems[i]);
                    slot->setPosition({ startX + i * (slotSize + spacing), currentY });
                    slot->setInteractive(false);
                    addChild(std::move(slot));
                }
                currentY += slotSize + 20.f;
            }
            else {
                auto emptyLabel = std::make_unique<TextWidget>(
                    *m_font, "The chest was empty.", 24, sf::Color::Red);
                sf::FloatRect lb = emptyLabel->getGlobalBounds();
                emptyLabel->setPosition({ centerX - lb.size.x / 2.f, currentY });
                addChild(std::move(emptyLabel));
                currentY += 30.f;
            };

            setSize({ getSize().x, getSize().y });
        }

    };

}