#pragma once
#include "GUI/windows/WindowBase.h"
#include "GUI/widgets/TextWidget.h"
#include <functional>

namespace RPG {
    class DefeatWindow : public WindowBase {
    public:
        /**
         * @param onCloseCallback Triggers when the 'X' button is clicked.
         */
        DefeatWindow(const sf::Font& font, std::function<void()> onCloseCallback);

        /**
         * @brief Builds the layout showing the gold penalty and displays the window.
         */
        void setResults(int goldLost);

        bool handleEvent(sf::RenderWindow& window, const sf::Event& event) override;

    private:
        const sf::Font& m_font;
        std::function<void()> m_onCloseCallback;

        void buildLayout(int goldLost);
    };
}