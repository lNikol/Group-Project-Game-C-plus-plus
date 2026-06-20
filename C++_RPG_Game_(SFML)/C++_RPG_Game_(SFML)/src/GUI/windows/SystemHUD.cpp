#include "SystemHUD.h"
#include "OpenWindowCommand.h"
#include "InventoryWindow.h"
#include "JournalWindow.h"
#include <cmath>

namespace RPG {

    SystemHUD::SystemHUD(const Spritesheet& iconSet, const sf::Font& font)
        : m_iconSet(iconSet), m_font(font), m_tooltip(font)
    {
        // Initialize System Buttons
        m_satchelBtn = std::make_unique<ActionSlot>(iconSet, font, sf::Vector2f(80.f, 80.f));
        m_journalBtn = std::make_unique<ActionSlot>(iconSet, font, sf::Vector2f(80.f, 80.f));
        m_bestiaryBtn = std::make_unique<ActionSlot>(iconSet, font, sf::Vector2f(80.f, 80.f));

        // Initialize Windows
        m_inventoryWindow = std::make_unique<InventoryWindow>(iconSet, font);
        m_journalWindow = std::make_unique<JournalWindow>(font);
        m_bestiaryWindow = std::make_unique<WindowBase>(font, sf::Vector2f(500.f, 600.f), "Bestiary");

        m_inventoryWindow->hide();
        m_journalWindow->hide();
        m_bestiaryWindow->hide();

        // Connect the Buttons to the Windows
        auto openBag = std::make_shared<OpenWindowCommand>(
            260, "Open Inventory",
            [this]() {
                this->m_inventoryWindow->toggle();
                if (m_inventoryWindow->isVisible()) {
                    m_journalWindow->hide();
                    m_bestiaryWindow->hide();
                }
            }
        );
        m_satchelBtn->setAbility(openBag);

        auto openJournal = std::make_shared<OpenWindowCommand>(
            225, "Journal",
            [this]() { 
                this->m_journalWindow->toggle(); 
                if (m_journalWindow->isVisible()) {
                    m_inventoryWindow->hide();
                    m_bestiaryWindow->hide();
                }
            }
        );
        m_journalBtn->setAbility(openJournal);

        auto openBestiary = std::make_shared<OpenWindowCommand>(
            224, "Bestiary",
            [this]() { 
                this->m_bestiaryWindow->toggle(); 
                if (m_bestiaryWindow->isVisible()) {
                    m_inventoryWindow->hide();
                    m_journalWindow->hide();
                }
            }
        );
        m_bestiaryBtn->setAbility(openBestiary);
    }

    void SystemHUD::onResize(const sf::Vector2u& newSize) {
        m_currentWindowSize = newSize;
        m_uiView = sf::View(sf::FloatRect({0.f, 0.f}, {(float)newSize.x, (float)newSize.y}));

        float w = (float)newSize.x;
        float h = (float)newSize.y;

        // Position Buttons Bottom-Right
        m_bestiaryBtn->setPosition(sf::Vector2f(w - 90.f, h - 90.f));
        m_journalBtn->setPosition(sf::Vector2f(w - 180.f, h - 90.f));
        m_satchelBtn->setPosition(sf::Vector2f(w - 270.f, h - 90.f));

        // Center Windows
        if (m_inventoryWindow) {
            sf::FloatRect invBounds = m_inventoryWindow->getGlobalBounds();
            m_inventoryWindow->setPosition({
                (w - invBounds.size.x) / 2.f,
                (h - invBounds.size.y) / 2.f
            });
        }
        
        if (m_journalWindow) {
            sf::FloatRect jrnBounds = m_journalWindow->getGlobalBounds();
            m_journalWindow->setPosition({
                (w - jrnBounds.size.x) / 2.f,
                (h - jrnBounds.size.y) / 2.f
            });
        }
        
        if (m_bestiaryWindow) {
            sf::FloatRect bstBounds = m_bestiaryWindow->getGlobalBounds();
            m_bestiaryWindow->setPosition({
                (w - bstBounds.size.x) / 2.f,
                (h - bstBounds.size.y) / 2.f
            });
        }
    }

    void SystemHUD::handleEvent(sf::RenderWindow& window, const sf::Event& event) {
        // Convert mouse position to UI view coordinates
        if (event.is<sf::Event::MouseMoved>()) {
            auto* ev = event.getIf<sf::Event::MouseMoved>();
            m_mousePos = window.mapPixelToCoords({ev->position.x, ev->position.y}, m_uiView);
        } else if (event.is<sf::Event::MouseButtonPressed>()) {
            auto* ev = event.getIf<sf::Event::MouseButtonPressed>();
            m_mousePos = window.mapPixelToCoords({ev->position.x, ev->position.y}, m_uiView);
        } else if (event.is<sf::Event::MouseButtonReleased>()) {
            auto* ev = event.getIf<sf::Event::MouseButtonReleased>();
            m_mousePos = window.mapPixelToCoords({ev->position.x, ev->position.y}, m_uiView);
        }

        sf::View oldView = window.getView();
        window.setView(m_uiView);

        // Hotkeys
        if (const auto* key = event.getIf<sf::Event::KeyPressed>()) {
            if (key->scancode == sf::Keyboard::Scancode::I) {
                m_inventoryWindow->toggle();
                if (m_inventoryWindow->isVisible()) {
                    m_journalWindow->hide();
                    m_bestiaryWindow->hide();
                }
            } else if (key->scancode == sf::Keyboard::Scancode::J) {
                m_journalWindow->toggle();
                if (m_journalWindow->isVisible()) {
                    m_inventoryWindow->hide();
                    m_bestiaryWindow->hide();
                }
            } else if (key->scancode == sf::Keyboard::Scancode::B) {
                m_bestiaryWindow->toggle();
                if (m_bestiaryWindow->isVisible()) {
                    m_inventoryWindow->hide();
                    m_journalWindow->hide();
                }
            }
        }

        if (m_inventoryWindow->handleEvent(window, event)) {
            window.setView(oldView);
            return;
        }
        if (m_journalWindow->handleEvent(window, event)) {
            window.setView(oldView);
            return;
        }
        if (m_bestiaryWindow->handleEvent(window, event)) {
            window.setView(oldView);
            return;
        }

        // Then buttons
        if (m_satchelBtn->handleEvent(window, event)) {
            window.setView(oldView);
            return;
        }
        if (m_journalBtn->handleEvent(window, event)) {
            window.setView(oldView);
            return;
        }
        if (m_bestiaryBtn->handleEvent(window, event)) {
            window.setView(oldView);
            return;
        }
        
        window.setView(oldView);
    }

    void SystemHUD::update(float dt) {
        m_inventoryWindow->update(dt);
        m_journalWindow->update(dt);
        m_bestiaryWindow->update(dt);
        
        m_satchelBtn->update(dt);
        m_journalBtn->update(dt);
        m_bestiaryBtn->update(dt);
        
        // Quest update highlight logic
        if (m_questHighlightTimer > 0.f) {
            m_questHighlightTimer -= dt;
            float scale = 1.0f + 0.15f * std::sin(m_questHighlightTimer * 10.f);
            m_journalBtn->setScale({scale, scale});
            
            if (m_questHighlightTimer <= 0.f) {
                m_journalBtn->setScale({1.f, 1.f});
            }
        }
    }
    
    void SystemHUD::notifyQuestUpdated() {
        m_questHighlightTimer = 2.0f; // Animate for 2 seconds
    }

    bool SystemHUD::isMouseOverUI() const {
        if (m_satchelBtn->getGlobalBounds().contains(m_mousePos)) return true;
        if (m_journalBtn->getGlobalBounds().contains(m_mousePos)) return true;
        if (m_bestiaryBtn->getGlobalBounds().contains(m_mousePos)) return true;

        if (m_inventoryWindow->isVisible() && m_inventoryWindow->getGlobalBounds().contains(m_mousePos)) return true;
        if (m_journalWindow->isVisible() && m_journalWindow->getGlobalBounds().contains(m_mousePos)) return true;
        if (m_bestiaryWindow->isVisible() && m_bestiaryWindow->getGlobalBounds().contains(m_mousePos)) return true;

        return false;
    }

    std::string SystemHUD::resolveTooltipText() const {
        if (auto text = m_inventoryWindow->getChildTooltipIfHovered()) return *text;
        if (auto text = m_journalWindow->getChildTooltipIfHovered()) return *text;
        if (auto text = m_bestiaryWindow->getChildTooltipIfHovered()) return *text;

        if (auto text = m_satchelBtn->getTooltipIfHovered()) return *text;
        if (auto text = m_journalBtn->getTooltipIfHovered()) return *text;
        if (auto text = m_bestiaryBtn->getTooltipIfHovered()) return *text;

        return "";
    }

    void SystemHUD::draw(sf::RenderTarget& target, sf::RenderStates states) const {
        sf::View oldView = target.getView();
        target.setView(m_uiView);

        target.draw(*m_satchelBtn);
        target.draw(*m_journalBtn);
        target.draw(*m_bestiaryBtn);

        target.draw(*m_inventoryWindow);
        target.draw(*m_journalWindow);
        target.draw(*m_bestiaryWindow);

        std::string tip = resolveTooltipText();
        if (!tip.empty()) {
            const_cast<Tooltip&>(m_tooltip).update(tip, m_mousePos, m_uiView.getSize());
            target.draw(m_tooltip);
        }

        target.setView(oldView);
    }

    void SystemHUD::closeAllWindows() {
        if (m_inventoryWindow) m_inventoryWindow->hide();
        if (m_journalWindow) m_journalWindow->hide();
        if (m_bestiaryWindow) m_bestiaryWindow->hide();
    }
}
