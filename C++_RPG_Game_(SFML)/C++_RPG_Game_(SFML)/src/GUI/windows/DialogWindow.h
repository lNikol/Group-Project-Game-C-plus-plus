#pragma once
#include "GUI/widgets/IGuiElement.h"
#include "GUI/widgets/NineSlice.h"
#include <SFML/Graphics.hpp>
#include <vector>

namespace RPG {

    class DialogWindow : public IGuiElement {
    public:
        DialogWindow(const sf::Font& font);

        void update(float dt) override;
        bool handleEvent(sf::RenderWindow& window, const sf::Event& event) override;
        sf::FloatRect getGlobalBounds() const override;

        void onResize(const sf::Vector2u& newSize);

    protected:
        void draw(sf::RenderTarget& target, sf::RenderStates states) const override;

    private:
        NineSlice m_background;
        sf::Text m_npcNameText;
        sf::Text m_dialogText;
        std::vector<sf::Text> m_responseTexts;
        
        const sf::Font& m_font;
        sf::Vector2f m_size;
        
        void refreshContent();
    };

}
