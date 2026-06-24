#include "WindowBase.h"
#include "worldmap/AssetManager.h"

namespace RPG {

    WindowBase::WindowBase(const sf::Font& font, const sf::Vector2f& size, const std::string& title)
        : m_titleText(font)
    {
        // Background NineSlice
        const sf::Texture* windowTex = AssetManager::getInstance().getTexture("ui_window");
        if (windowTex) {
            m_background.setTexture(*windowTex, 32, 1.0f); // 32px corners, scaled 1x to 32px visually
        }
        m_background.setSize(size);

        // Title Text
        m_titleText.setString(title);
        m_titleText.setCharacterSize(32);
        m_titleText.setFillColor(sf::Color::White);
        m_titleText.setPosition({ 10.f, 4.f });

        // Close Button (Top Right)
        const sf::Texture* closeTex = AssetManager::getInstance().getTexture("close_button");
        if (closeTex) {
            m_closeBtnSprite.emplace(*closeTex);
        }
        float btnSize = 24.f;
        if (m_closeBtnSprite.has_value()) {
            sf::FloatRect texBounds = m_closeBtnSprite->getLocalBounds();
            if (texBounds.size.x > 0) {
                m_closeBtnSprite->setScale({ btnSize / texBounds.size.x, btnSize / texBounds.size.y });
            }
            m_closeBtnSprite->setPosition({ size.x - btnSize - 4.f, 3.f }); // 4px padding
        }
    }

    void WindowBase::setSize(const sf::Vector2f& size) {
        m_background.setSize(size);
        
        float btnSize = 24.f;
        if (m_closeBtnSprite.has_value()) {
            m_closeBtnSprite->setPosition({ size.x - btnSize - 4.f, 3.f });
        }
    }

    sf::Vector2f WindowBase::getSize() const {
        return m_background.getSize();
    }

    void WindowBase::addChild(std::unique_ptr<IGuiElement> widget) {
        widget->setParent(this);
        m_children.push_back(std::move(widget));
    }

    void WindowBase::show() { m_isVisible = true; }
    void WindowBase::hide() { m_isVisible = false; }
    void WindowBase::toggle() { m_isVisible = !m_isVisible; }

    void WindowBase::update(float dt) {
        if (!m_isVisible) return;

        for (auto& child : m_children) {
            child->update(dt);
        }
    }

    sf::FloatRect WindowBase::getGlobalBounds() const {
        return getAbsoluteTransform().transformRect(m_background.getLocalBounds());
    }

    bool WindowBase::handleEvent(sf::RenderWindow& window, const sf::Event& event) {
        if (!m_isVisible) return false;

        // Convert mouse to view coordinates for checks
        sf::Vector2f mousePos;
        if (const auto* move = event.getIf<sf::Event::MouseMoved>()) {
            mousePos = window.mapPixelToCoords(move->position);

            // Check Hover on Close Button
            if (m_closeBtnSprite.has_value()) {
                sf::FloatRect closeGlobal = getTransform().transformRect(m_closeBtnSprite->getGlobalBounds());
                m_closeBtnHovered = closeGlobal.contains(mousePos);
                if (m_closeBtnHovered) {
                    m_closeBtnSprite->setColor(sf::Color(200, 200, 200));
                } else {
                    m_closeBtnSprite->setColor(sf::Color::White);
                }
            }
        }
        else if (const auto* click = event.getIf<sf::Event::MouseButtonPressed>()) {
            mousePos = window.mapPixelToCoords(click->position);
        }

        // Handle Close Button Click
        if (const auto* click = event.getIf<sf::Event::MouseButtonPressed>()) {
            if (click->button == sf::Mouse::Button::Left && m_closeBtnHovered) {
                hide();
                return true; // Consumed
            }
        }

        // Pass to Children
        for (auto& child : m_children) {
            if (child->handleEvent(window, event)) {
                return true; // Child consumed it
            }
        }

        // Swallow clicks on the background
        if (const auto* click = event.getIf<sf::Event::MouseButtonPressed>()) {
            if (getGlobalBounds().contains(mousePos)) {
                return true;
            }
        }

        return false;
    }

    std::optional<std::string> WindowBase::getChildTooltipIfHovered() const {
        if (!m_isVisible) return std::nullopt;

        // Check children first
        for (const auto& child : m_children) {
            if (auto text = child->getTooltipIfHovered()) {
                return text;
            }
        }
        return std::nullopt;
    }

    void WindowBase::draw(sf::RenderTarget& target, sf::RenderStates states) const {
        if (!m_isVisible) return;

        states.transform *= getTransform();

        // Draw Frame
        target.draw(m_background, states);
        
        target.draw(m_titleText, states);

        // Draw Close Button
        if (m_closeBtnSprite.has_value()) {
            target.draw(*m_closeBtnSprite, states);
        }

        // Draw Children (Buttons, Slots)
        for (const auto& child : m_children) {
            target.draw(*child, states); // Child draws itself
        }
    }
}