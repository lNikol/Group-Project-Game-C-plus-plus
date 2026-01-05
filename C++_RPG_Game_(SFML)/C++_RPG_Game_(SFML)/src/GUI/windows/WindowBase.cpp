#include "WindowBase.h"

namespace RPG {

    WindowBase::WindowBase(const sf::Font& font, const sf::Vector2f& size, const std::string& title)
        : m_titleText(font), m_closeBtnText(font)
    {
        // Background
        m_background.setSize(size);
        m_background.setFillColor(sf::Color(30, 30, 30, 240)); // Semi-transparent dark
        m_background.setOutlineColor(sf::Color(200, 200, 200));
        m_background.setOutlineThickness(2.f);

        // Title Bar
        float titleHeight = 30.f;
        m_titleBar.setSize({ size.x, titleHeight });
        m_titleBar.setFillColor(sf::Color(50, 50, 80)); // Slightly blueish header

        // Title Text
        m_titleText.setString(title);
        m_titleText.setCharacterSize(18);
        m_titleText.setFillColor(sf::Color::White);
        m_titleText.setPosition({ 10.f, 4.f });

        // Close Button (Top Right)
        float btnSize = 24.f;
        m_closeBtnRect.setSize({ btnSize, btnSize });
        m_closeBtnRect.setFillColor(sf::Color(150, 50, 50)); // Red
        m_closeBtnRect.setOutlineColor(sf::Color::White);
        m_closeBtnRect.setOutlineThickness(1.f);
        m_closeBtnRect.setPosition({ size.x - btnSize - 4.f, 3.f }); // 4px padding

        m_closeBtnText.setString("X");
        m_closeBtnText.setCharacterSize(16);
        m_closeBtnText.setFillColor(sf::Color::White);
        // Center 'X'
        sf::FloatRect textBounds = m_closeBtnText.getLocalBounds();
        m_closeBtnText.setOrigin(textBounds.position + textBounds.size / 2.f);
        m_closeBtnText.setPosition(m_closeBtnRect.getPosition() + sf::Vector2f(btnSize / 2.f, btnSize / 2.f));
    }

    void WindowBase::addChild(std::unique_ptr<IGuiElement> widget) {
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
        // Return the bounds of the background transformed by current position
        sf::FloatRect bounds = m_background.getGlobalBounds();
        return getTransform().transformRect(bounds);
    }

    bool WindowBase::handleEvent(const sf::RenderWindow& window, const sf::Event& event) {
        if (!m_isVisible) return false;

        // Convert mouse to view coordinates for checks
        sf::Vector2f mousePos;
        if (const auto* move = event.getIf<sf::Event::MouseMoved>()) {
            mousePos = window.mapPixelToCoords(move->position);

            // Check Hover on Close Button
            sf::FloatRect closeGlobal = getTransform().transformRect(m_closeBtnRect.getGlobalBounds());
            m_closeBtnHovered = closeGlobal.contains(mousePos);
            m_closeBtnRect.setFillColor(m_closeBtnHovered ? sf::Color(200, 50, 50) : sf::Color(150, 50, 50));
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
        target.draw(m_titleBar, states);
        target.draw(m_titleText, states);

        // Draw Close Button
        target.draw(m_closeBtnRect, states);
        target.draw(m_closeBtnText, states);

        // Draw Children (Buttons, Slots)
        for (const auto& child : m_children) {
            target.draw(*child, states); // Child draws itself
        }
    }
}