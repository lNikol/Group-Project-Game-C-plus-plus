#pragma once
#include "GUI/windows/WindowBase.h"
#include "core/QuestManager.h"
#include <vector>

namespace RPG {
    class JournalWindow : public WindowBase {
    public:
        JournalWindow(const sf::Font& font);
        
        void setSize(const sf::Vector2f& size) override;

        void update(float dt) override;

    private:
        const sf::Font& m_font;
        void refreshQuestList();
        std::string wrapText(const std::string& str, float width, unsigned int charSize);

        struct QuestCard {
            NineSlice background;
            std::vector<std::unique_ptr<sf::Text>> texts;
        };
        std::vector<QuestCard> m_questCards;
        std::vector<std::unique_ptr<sf::Text>> m_questTexts;
    protected:
        void draw(sf::RenderTarget& target, sf::RenderStates states) const override;
    };
}
