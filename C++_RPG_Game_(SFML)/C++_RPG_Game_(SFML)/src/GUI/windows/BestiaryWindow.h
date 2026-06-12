#pragma once
#include "GUI/windows/WindowBase.h"
#include <SFML/Graphics.hpp>
#include <vector>
#include <string>
#include <optional>

namespace RPG {

    class BestiaryWindow : public WindowBase {
    public:
        explicit BestiaryWindow(const sf::Font& font);
        void draw(sf::RenderTarget& target, sf::RenderStates states) const override;

    private:
        void loadBestiaryData();

        struct BestiaryEntry {
            std::string name;
            std::optional<sf::Sprite> sprite;
            float hp, mp, stamina;
            int initiative;
            std::vector<std::string> abilities;    // names of abilities
            float critChance = 0.f;
            float doubleTurnChance = 0.f;
        };

        std::vector<BestiaryEntry> m_entries;
        const sf::Font& m_font;
    };

}