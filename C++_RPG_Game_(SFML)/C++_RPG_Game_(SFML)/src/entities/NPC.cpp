#include "NPC.h"
#include "core/Constants.h" 

namespace RPG {

    NPC::NPC(sf::Vector2f pos, std::string texKey, uint8_t tileIdx, std::string npcName, bool isLeader, FactionID target )
        : position(pos), textureKey(texKey), tileIndex(tileIdx), name(npcName), 
        isFactionLeader(isLeader), interactionRadius(GameConfig::TILE_SIZE * 2), targetFaction(target)
    {
    }

    void NPC::draw(sf::RenderWindow& window, const AssetManager& sm) {
        const sf::Texture* texture = sm.getSpritesheet(textureKey);
        const uint8_t ts = GameConfig::TILE_SIZE;

        if (texture) {
            sf::Sprite sprite(*texture);

            uint8_t columns = texture->getSize().x / ts;
            int16_t x = (tileIndex % columns) * ts;
            int16_t y = (tileIndex / columns) * ts;

            sprite.setTextureRect(sf::IntRect({ x, y }, { ts, ts }));
            sprite.setPosition(position);

            if (isFactionLeader) {
                sprite.setColor(sf::Color(255, 255, 200)); 
            }

            window.draw(sprite);
        }
        else {
            // no texture
            sf::RectangleShape debugRect(sf::Vector2f(static_cast<float>(ts), static_cast<float>(ts)));
            debugRect.setPosition(position);

            if (isFactionLeader) {
                debugRect.setFillColor(sf::Color::Yellow);
                debugRect.setOutlineColor(sf::Color::White);
                debugRect.setOutlineThickness(1.0f);
            }
            else {
                debugRect.setFillColor(sf::Color(255, 165, 0));
            }

            window.draw(debugRect);
        }
    }



    sf::Vector2f NPC::getPosition() const { return position; }
    float NPC::getInteractionRadius() const { return interactionRadius; }
    const std::string& NPC::getName() const { return name; }
    bool NPC::getIsFactionLeader() const { return isFactionLeader; }
    FactionID NPC::getTargetFaction() const { return targetFaction; }


    void NPC::setPosition(sf::Vector2f pos) {
        position = pos;
    }

}