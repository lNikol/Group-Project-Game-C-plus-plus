#include "BestiaryWindow.h"
#include "worldmap/AssetManager.h"
#include <fstream>
#include <iostream>
#include <sstream>
#include <external/json.hpp>

using json = nlohmann::json;

namespace RPG {

    BestiaryWindow::BestiaryWindow(const sf::Font& font)
        : WindowBase(font, sf::Vector2f(500.f, 600.f), "Bestiary")
        , m_font(font)
    {
        loadBestiaryData();
    }

    void BestiaryWindow::loadBestiaryData()
    {
        const std::vector<std::string> wantedIds = { "enemy_skeleton", "enemy_djinn", "enemy_orc", "enemy_shadow", "boss_demon" };

        std::ifstream file("assets/jsons/units.json");
        if (!file.is_open()) {
            std::cerr << "Bestiary: Could not open units.json" << std::endl;
            return;
        }

        json root;
        try {
            file >> root;
        }
        catch (const json::parse_error& e) {
            std::cerr << "Bestiary: JSON parse error: " << e.what() << std::endl;
            return;
        }

        if (!root.contains("units") || !root["units"].is_array()) {
            std::cerr << "Bestiary: 'units' array missing" << std::endl;
            return;
        }

        for (const auto& item : root["units"]) {
            std::string id = item.value("id", "");
            if (std::find(wantedIds.begin(), wantedIds.end(), id) == wantedIds.end())
                continue;

            BestiaryEntry entry;
            entry.name = item.value("name", "Unknown");
            entry.hp = item.value("hp", 0.f);
            entry.mp = item.value("mp", 0.f);
            entry.stamina = item.value("stamina", 0.f);
            entry.initiative = item.value("initiative", 0);
            entry.critChance = item.value("critChance", 0.f);
            entry.doubleTurnChance = item.value("doubleTurnChance", 0.f);

            // Abilities array
            if (item.contains("abilities") && item["abilities"].is_array()) {
                for (const auto& ab : item["abilities"]) {
                    entry.abilities.push_back(ab.get<std::string>());
                }
            }

            // Sprite
            std::string spritesheetId = item.value("spritesheetId", "BattleEnemies");
            const Spritesheet* sheet = AssetManager::getInstance().getSpritesheet(spritesheetId);
            if (sheet && item.contains("textureRect") && item["textureRect"].is_array()) {
                sf::IntRect rect(
                    { item["textureRect"][0], item["textureRect"][1] },
                    { item["textureRect"][2], item["textureRect"][3] }
                );
                entry.sprite.emplace(*sheet, rect);
                entry.sprite->setScale({ 2.f, 2.f });
            }

            m_entries.push_back(std::move(entry));
        }
    }

    void BestiaryWindow::draw(sf::RenderTarget& target, sf::RenderStates states) const
    {
        WindowBase::draw(target, states);
        if (!m_isVisible) return;

        const sf::FloatRect bounds = getGlobalBounds();
        float startX = bounds.position.x + 10.f;
        float startY = bounds.position.y + 40.f;
        const float spacing = 120.f;

        float yOffset = startY;
        for (const auto& entry : m_entries) {
            // --- Sprite ---
            if (entry.sprite.has_value()) {
                sf::Sprite spriteCopy = *entry.sprite;
                spriteCopy.setPosition({ startX, yOffset });
                target.draw(spriteCopy, states);
            }

            // --- Name ---
            sf::Text nameText(m_font);
            nameText.setString(entry.name);
            nameText.setCharacterSize(18);
            nameText.setPosition({ startX + 80.f, yOffset });
            target.draw(nameText, states);

            // --- Basic Stats ---
            sf::Text statsText(m_font);
            std::stringstream ss;
            ss << "HP: " << static_cast<int>(entry.hp)
                << "  MP: " << static_cast<int>(entry.mp)
                << "  Stam: " << static_cast<int>(entry.stamina)
                << "  Init: " << entry.initiative;
            statsText.setString(ss.str());
            statsText.setCharacterSize(14);
            statsText.setPosition({ startX + 80.f, yOffset + 24.f });
            target.draw(statsText, states);

            // --- Abilities ---
            if (!entry.abilities.empty()) {
                sf::Text abilityText(m_font);
                std::stringstream abs;
                abs << "Abilities: ";
                for (size_t i = 0; i < entry.abilities.size(); ++i) {
                    if (i > 0) abs << ", ";
                    abs << entry.abilities[i];
                }
                abilityText.setString(abs.str());
                abilityText.setCharacterSize(13);
                abilityText.setFillColor(sf::Color(200, 200, 200)); // light gray
                abilityText.setPosition({ startX + 80.f, yOffset + 42.f });
                target.draw(abilityText, states);
            }

            // --- Crit & Double Turn Chance ---
            if (entry.critChance > 0.f || entry.doubleTurnChance > 0.f) {
                sf::Text extraText(m_font);
                std::stringstream ext;
                ext.precision(0);
                ext << std::fixed;
                ext << "Crit: " << entry.critChance * 100 << "%"
                    << "  DblTurn: " << entry.doubleTurnChance * 100 << "%";
                extraText.setString(ext.str());
                extraText.setCharacterSize(13);
                extraText.setFillColor(sf::Color(200, 200, 200));
                // Position below abilities (if any) or below stats
                float yPos = yOffset + 42.f;
                if (!entry.abilities.empty()) yPos += 16.f; // move down to avoid overlap
                extraText.setPosition({ startX + 80.f, yPos });
                target.draw(extraText, states);
            }

            yOffset += spacing;
        }
    }

}