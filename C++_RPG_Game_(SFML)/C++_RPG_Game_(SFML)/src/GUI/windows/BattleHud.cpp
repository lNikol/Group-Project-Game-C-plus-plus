#include "BattleHUD.h"
#include "OpenWindowCommand.h"
#include <cmath>

#include "InventoryWindow.h"
#include "BestiaryWindow.h"
namespace RPG {

    BattleHUD::BattleHUD(const Spritesheet& iconSet, const sf::Font& font)
        : m_iconSet(iconSet), m_font(font), m_tooltip(font)
    {
        // Vitals (Bottom Left)
        // Size: 20px wide, 120px tall
        sf::Vector2f barSize(20.f, 120.f);
        m_hpBar = std::make_unique<StatBar>(barSize, sf::Color(200, 50, 50)); // Red
        m_mpBar = std::make_unique<StatBar>(barSize, sf::Color(50, 50, 200)); // Blue
        m_staminaBar = std::make_unique<StatBar>(barSize, sf::Color(50, 200, 50)); // Green

        // Ability Grid (Bottom Center)
        // 2 rows of 9 columns = 18 slots
        for (int i = 0; i < 18; ++i) {
            auto slot = std::make_unique<ActionSlot>(iconSet, font, sf::Vector2f(40.f, 40.f));
            m_actionGrid.push_back(std::move(slot));
        }
    }

    void BattleHUD::setCombatActor(ICombatActor* actor) {
        m_actor = actor;
    }

    void BattleHUD::onResize(const sf::Vector2u& newSize) {
        m_currentWindowSize = newSize;

        sf::Vector2f logicalSize = Window::getLogicalSize(newSize);
        m_uiView.setSize(logicalSize);
        m_uiView.setCenter({ logicalSize.x / 2.f, logicalSize.y / 2.f });

        float padding = 10.f;

        // Anchor Vitals (Bottom Left)
        float barH = 120.f; //height
        float startY = logicalSize.y - barH - padding;

        m_hpBar->setPosition({ padding, startY });
        m_mpBar->setPosition({ padding + 25.f, startY });      // +25px offset
        m_staminaBar->setPosition({ padding + 50.f, startY }); // +50px offset

        // Anchor Action Grid (Bottom Center)
        // 2 Rows, 9 Cols. Slot Size 40. Spacing 4.
        float slotSize = 40.f;
        float spacing = 4.f;
        int cols = 9;

        float gridWidth = (cols * slotSize) + ((cols - 1) * spacing);
        float gridHeight = (2 * slotSize) + spacing;

        float gridStartX = (logicalSize.x / 2.f) - (gridWidth / 2.f); // Center X
        float gridStartY = logicalSize.y - gridHeight - padding;      // Bottom Y

        for (int i = 0; i < 18; ++i) {
            int r = i / cols; // Row (0 to 1)
            int c = i % cols; // Col (0 to 8)

            float x = gridStartX + c * (slotSize + spacing);
            float y = gridStartY + r * (slotSize + spacing);

            m_actionGrid[i]->setPosition({ x, y });
        }

    }

    void BattleHUD::handleEvent(sf::RenderWindow& window, const sf::Event& event) {
        // Update Mouse position
        if (const auto* move = event.getIf<sf::Event::MouseMoved>()) {
            m_mousePos = window.mapPixelToCoords(move->position, m_uiView);
        }
        sf::View previousView = window.getView();
        window.setView(m_uiView);

        // Pass to Buttons
        for (auto& slot : m_actionGrid) {
            if (slot->handleEvent(window, event)) return;
        }

        // Vitals for tooltips
        m_hpBar->handleEvent(window, event);
        m_mpBar->handleEvent(window, event);
        m_staminaBar->handleEvent(window, event);

        window.setView(previousView);
    }

    void BattleHUD::update(float dt) {
        // Sync Vitals
        if (m_actor) {
            Vitals v = m_actor->getVitals();
            m_hpBar->setValues(v.hp, v.maxHp);
            m_mpBar->setValues(v.mp, v.maxMp);
            m_staminaBar->setValues(v.stamina, v.maxStamina);

            // Sync Hotbar
            for (int i = 0; i < 18; ++i) {
                auto ability = m_actor->getHotbarAbility(i);
                if (ability != m_actionGrid[i]->getAbility()) {
                    m_actionGrid[i]->setAbility(ability);
                }

                // Update cooldowns/charges
                m_actionGrid[i]->update(dt);
            }
        }
    }


    bool BattleHUD::isMouseOverUI() const {
        return !resolveTooltipText().empty();
    }

    std::string BattleHUD::resolveTooltipText() const {
        // Ability Grid
        for (const auto& slot : m_actionGrid) {
            if (auto text = slot->getTooltipIfHovered()) return *text;
        }

        // Vitals
        if (auto text = m_hpBar->getTooltipIfHovered()) return *text;
        if (auto text = m_mpBar->getTooltipIfHovered()) return *text;
        if (auto text = m_staminaBar->getTooltipIfHovered()) return *text;

        return ""; // Nothing hovered
    }

    void BattleHUD::draw(sf::RenderTarget& target, sf::RenderStates states) const {
        // Switch View to UI Mode
        sf::View originalView = target.getView();
        target.setView(m_uiView);

        //sf::CircleShape debugMouse(5.f);
        //debugMouse.setFillColor(sf::Color::Yellow);
        //debugMouse.setPosition(m_mousePos);
        //target.draw(debugMouse);

        // Draw Bottom Layer
        target.draw(*m_hpBar);
        target.draw(*m_mpBar);
        target.draw(*m_staminaBar);

        for (const auto& slot : m_actionGrid) {
            target.draw(*slot);
        }

        std::string tipText = resolveTooltipText();

        const_cast<Tooltip&>(m_tooltip).update(tipText, m_mousePos, m_uiView.getSize());

        target.draw(m_tooltip);
        target.setView(originalView);
    }

    void BattleHUD::closeAllWindows() {
    }
}