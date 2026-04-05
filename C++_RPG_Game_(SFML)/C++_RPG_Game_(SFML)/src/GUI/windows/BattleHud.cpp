#include "BattleHUD.h"
#include "OpenWindowCommand.h" // If you made this earlier
#include <cmath>

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
            // You can configure dummy hotkeys or indices here
            m_actionGrid.push_back(std::move(slot));
        }

        // Initialize System Buttons (Bottom Right)
        m_satchelBtn = std::make_unique<ActionSlot>(iconSet, font, sf::Vector2f(80.f, 80.f));
        m_journalBtn = std::make_unique<ActionSlot>(iconSet, font, sf::Vector2f(80.f, 80.f));

        // Initialize Windows (Hidden)
        m_inventoryWindow = std::make_unique<WindowBase>(font, sf::Vector2f(400.f, 300.f), "Inventory");
        m_journalWindow = std::make_unique<WindowBase>(font, sf::Vector2f(300.f, 400.f), "Journal");

        // Connect the Buttons to the Windows
        auto openBag = std::make_shared<OpenWindowCommand>(
            260, "Open Inventory", // icon index
            [this]() {
                this->m_inventoryWindow->toggle();
                if (m_inventoryWindow->isVisible()) m_journalWindow->hide();
            }
        );
        m_satchelBtn->setAbility(openBag);

        auto openJournal = std::make_shared<OpenWindowCommand>(
            225, "Journal",
            [this]() { 
                this->m_journalWindow->toggle(); 
                if (m_journalWindow->isVisible()) m_inventoryWindow->hide();
            }
        );
        m_journalBtn->setAbility(openJournal);
    }

    void BattleHUD::setCombatActor(ICombatActor* actor) {
        m_actor = actor;
    }

    void BattleHUD::onResize(const sf::Vector2u& newSize) {
        m_currentWindowSize = newSize;

        m_uiView.setSize({ (float)newSize.x, (float)newSize.y });
        m_uiView.setCenter({ newSize.x / 2.f, newSize.y / 2.f });

        float padding = 10.f;

        // Anchor Vitals (Bottom Left)
        float barH = 120.f; //height
        float startY = newSize.y - barH - padding;

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

        float gridStartX = (newSize.x / 2.f) - (gridWidth / 2.f); // Center X
        float gridStartY = newSize.y - gridHeight - padding;      // Bottom Y

        for (int i = 0; i < 18; ++i) {
            int r = i / cols; // Row (0 to 1)
            int c = i % cols; // Col (0 to 8)

            float x = gridStartX + c * (slotSize + spacing);
            float y = gridStartY + r * (slotSize + spacing);

            m_actionGrid[i]->setPosition({ x, y });
        }

        // Anchor System Buttons (Bottom Right).
        float btnSize = 80.f;

        // Journal (Far Right)
        m_journalBtn->setPosition({ newSize.x - btnSize - padding, newSize.y - btnSize - padding });

        // Inventory (Left of Journal)
        m_satchelBtn->setPosition({ newSize.x - (btnSize * 2) - padding - spacing, newSize.y - btnSize - padding });

        // Center Windows
        // Inventory
        sf::FloatRect invBounds = m_inventoryWindow->getGlobalBounds();
        m_inventoryWindow->setPosition({
            (newSize.x - invBounds.size.x) / 2.f,
            (newSize.y - invBounds.size.y) / 2.f
            });

        // Journal
        sf::FloatRect jrnBounds = m_journalWindow->getGlobalBounds();
        m_journalWindow->setPosition({
            (newSize.x - jrnBounds.size.x) / 2.f,
            (newSize.y - jrnBounds.size.y) / 2.f
            });
    }

    void BattleHUD::handleEvent(sf::RenderWindow& window, const sf::Event& event) {
        // Update Mouse position
        if (const auto* move = event.getIf<sf::Event::MouseMoved>()) {
            m_mousePos = window.mapPixelToCoords(move->position, m_uiView);
        }
        sf::View previousView = window.getView();
        window.setView(m_uiView);
        // Pass to Windows first
        if (m_inventoryWindow->handleEvent(window, event)) return;
        if (m_journalWindow->handleEvent(window, event)) return;

        // Pass to Buttons

        if (m_satchelBtn->handleEvent(window, event)) return;
        if (m_journalBtn->handleEvent(window, event)) return;

        for (auto& slot : m_actionGrid) {
            if (slot->handleEvent(window, event)) return;
        }

        // Vitals for tooltips
        m_hpBar->handleEvent(window, event);
        m_mpBar->handleEvent(window, event);
        m_staminaBar->handleEvent(window, event);

        window.setView(m_uiView);
    }

    void BattleHUD::update(float dt) {
        //updates
        m_inventoryWindow->update(dt);
        m_journalWindow->update(dt);
        m_satchelBtn->update(dt);
        m_journalBtn->update(dt);

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
        // Check Windows
        if (auto text = m_inventoryWindow->getChildTooltipIfHovered()) return *text;
        if (auto text = m_journalWindow->getChildTooltipIfHovered()) return *text;

        // Buttons
        if (auto text = m_satchelBtn->getTooltipIfHovered()) return *text;
        if (auto text = m_journalBtn->getTooltipIfHovered()) return *text;

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
        // Draw Bottom Layer
        target.draw(*m_hpBar);
        target.draw(*m_mpBar);
        target.draw(*m_staminaBar);

        for (const auto& slot : m_actionGrid) {
            target.draw(*slot);
        }

        target.draw(*m_satchelBtn);
        target.draw(*m_journalBtn);

        // Draw Top Layer
        target.draw(*m_inventoryWindow);
        target.draw(*m_journalWindow);

        std::string tipText = resolveTooltipText();

        const_cast<Tooltip&>(m_tooltip).update(tipText, m_mousePos, m_uiView.getSize());

        target.draw(m_tooltip);
        target.setView(originalView);
    }
}