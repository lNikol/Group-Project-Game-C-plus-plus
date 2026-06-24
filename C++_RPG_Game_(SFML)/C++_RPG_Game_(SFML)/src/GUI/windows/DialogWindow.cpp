#include "DialogWindow.h"
#include "core/DialogManager.h"
#include "worldmap/AssetManager.h"
#include "core/Constants.h"
#include <iostream>

namespace RPG {

    DialogWindow::DialogWindow(const sf::Font& font)
        : m_font(font), m_npcNameText(font), m_dialogText(font)
    {
        // Use a placeholder UI texture for NineSlice background
        if (const sf::Texture* tex = AssetManager::getInstance().getTexture("ui_window")) {
            m_background.setTexture(*tex, 32, 1.0f);
        }

        m_npcNameText.setCharacterSize(32);
        m_npcNameText.setFillColor(sf::Color::White);

        m_dialogText.setCharacterSize(16);
        m_dialogText.setFillColor(sf::Color::White);

        onResize(sf::Vector2u(1280, 720)); // arbitrary default
    }

    void DialogWindow::onResize(const sf::Vector2u& newSize) {
        sf::Vector2f logicalSize = Window::getLogicalSize(newSize);
        m_size = sf::Vector2f(logicalSize.x, 200.f);
        m_background.setSize(m_size);

        // Position it at the bottom of the screen
        float x = 0.f;
        float y = logicalSize.y - m_size.y;
        setPosition({x, y});
    }

    void DialogWindow::update(float dt) {
        refreshContent();
    }

    void DialogWindow::refreshContent() {
        if (!DialogManager::getInstance().isActive()) return;

        auto* tree = DialogManager::getInstance().getCurrentDialog();
        auto* node = DialogManager::getInstance().getCurrentNode();

        if (tree && node) {
            m_npcNameText.setString(tree->npcName);
            m_dialogText.setString(node->text);

            if (!tree->npcPortrait.empty()) {
                const Spritesheet* portraitTex = AssetManager::getInstance().getSpritesheet(tree->npcPortrait);
                if (!portraitTex) {
                    portraitTex = &AssetManager::getInstance().getTest(tree->npcPortrait);
                }
                if (portraitTex && portraitTex->getSize().x > 0) {
                    m_npcPortrait.emplace(*portraitTex);
                    sf::Vector2i offset = {0, 0};
                    if (tree->npcPortrait == "vampire" || tree->npcPortrait == "ork" || tree->npcPortrait == "knight" || tree->npcPortrait == "player") {
                        offset = {16, 16};
                    } else if (tree->npcPortrait == "npc") {
                        offset = {8, 16};
                    }
                    m_npcPortrait->setTextureRect(sf::IntRect(offset, {32, 32}));
                    m_npcPortrait->setScale({4.f, 4.f});
                } else {
                    m_npcPortrait.reset();
                }
            } else {
                m_npcPortrait.reset();
            }

            m_responseTexts.clear();
            for (size_t i = 0; i < node->responses.size(); ++i) {
                sf::Text t(m_font);
                t.setCharacterSize(16);
                t.setFillColor(sf::Color(200, 200, 200));
                std::string prefix = std::to_string(i + 1) + ". ";
                t.setString(prefix + node->responses[i].text);
                m_responseTexts.push_back(t);
            }
        }

        // Layout
        float textStartX = 20.f;
        if (m_npcPortrait.has_value()) {
            m_npcPortrait->setPosition({20.f, 36.f});
            textStartX = 168.f; // 20 + 128 + 20
        }

        m_npcNameText.setPosition({textStartX, 15.f});
        m_dialogText.setPosition({textStartX, 50.f});

        float currentY = 100.f;
        for (auto& t : m_responseTexts) {
            t.setPosition({textStartX, currentY});
            currentY += 24.f;
        }
    }

    bool DialogWindow::handleEvent(sf::RenderWindow& window, const sf::Event& event) {
        if (!DialogManager::getInstance().isActive()) return false;

        if (const auto* key = event.getIf<sf::Event::KeyPressed>()) {
            if (key->scancode >= sf::Keyboard::Scancode::Num1 && key->scancode <= sf::Keyboard::Scancode::Num9) {
                int index = static_cast<int>(key->scancode) - static_cast<int>(sf::Keyboard::Scancode::Num1);
                auto* node = DialogManager::getInstance().getCurrentNode();
                if (node && index < node->responses.size()) {
                    DialogManager::getInstance().selectResponse(index);
                    return true;
                }
            }
        }
        // Consume all input while dialog is active so the player cannot move or click other UI
        // Actually, only returning true if we want to block it. 
        // Returning true here blocks all events (including window close events!), so we need to be careful.
        // We only block keyboard/mouse events.
        if (event.is<sf::Event::KeyPressed>() || event.is<sf::Event::KeyReleased>() ||
            event.is<sf::Event::MouseButtonPressed>() || event.is<sf::Event::MouseButtonReleased>() ||
            event.is<sf::Event::MouseWheelScrolled>()) {
            return true;
        }
        return false;
    }

    sf::FloatRect DialogWindow::getGlobalBounds() const {
        sf::Transform tf = getAbsoluteTransform();
        return tf.transformRect(sf::FloatRect(sf::Vector2f(0.f, 0.f), m_size));
    }

    void DialogWindow::draw(sf::RenderTarget& target, sf::RenderStates states) const {
        if (!DialogManager::getInstance().isActive()) return;

        states.transform *= getTransform();
        target.draw(m_background, states);
        if (m_npcPortrait.has_value()) {
            target.draw(*m_npcPortrait, states);
        }
        target.draw(m_npcNameText, states);
        target.draw(m_dialogText, states);

        for (const auto& t : m_responseTexts) {
            target.draw(t, states);
        }
    }
}
