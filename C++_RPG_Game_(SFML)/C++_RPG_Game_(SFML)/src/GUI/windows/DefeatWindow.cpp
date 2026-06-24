#include "DefeatWindow.h"

namespace RPG {
    DefeatWindow::DefeatWindow(const sf::Font& font, std::function<void()> onCloseCallback)
        : WindowBase(font, sf::Vector2f(300.f, 150.f), "Defeat..."),
        m_font(font),
        m_onCloseCallback(onCloseCallback)
    {
    }

    bool DefeatWindow::handleEvent(sf::RenderWindow& window, const sf::Event& event) {
        bool consumed = WindowBase::handleEvent(window, event);

        if (!m_isVisible && m_onCloseCallback) {
            m_onCloseCallback();
            m_onCloseCallback = nullptr;
        }

        return consumed;
    }

    void DefeatWindow::setResults(int goldLost) {
        buildLayout(goldLost);
        show();
    }

    void DefeatWindow::buildLayout(int goldLost) {
        m_children.clear();

        float currentY = 50.f;
        float centerX = getSize().x / 2.f;

        auto lossText = std::make_unique<TextWidget>(m_font, "Your party has fallen.", 26, sf::Color(200, 50, 50));
        sf::FloatRect bounds1 = lossText->getGlobalBounds();
        lossText->setPosition({ centerX - (bounds1.size.x / 2.f), currentY });
        addChild(std::move(lossText));

        currentY += 30.f;

        auto goldText = std::make_unique<TextWidget>(m_font, "Gold Lost: " + std::to_string(goldLost), 22, sf::Color(150, 150, 150));
        sf::FloatRect bounds2 = goldText->getGlobalBounds();
        goldText->setPosition({ centerX - (bounds2.size.x / 2.f), currentY });
        addChild(std::move(goldText));
    }
}