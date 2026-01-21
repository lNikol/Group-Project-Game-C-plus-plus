#include "Player.h"
#include "core/Constants.h"
#include <cmath>

namespace RPG {

    Player::Player(float startX, float startY, std::string name) : position(startX, startY), name(name) {
        hotbar.resize(hotBarSize, nullptr);
        inventory.resize(inventorySize, nullptr);
    }
    Player::Player(sf::Vector2f pos, std::string name) : Player(pos.x, pos.y, name){}

    sf::Vector2f Player::getPosition() const { return position; }

    sf::Vector2f Player::getInputDirection(const sf::RenderWindow& window) const {

        sf::Vector2f dir(0.0f, 0.0f);

        if (!window.hasFocus()) return dir;

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W) || sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Up))    dir.y -= 1.0f;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S) || sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Down))  dir.y += 1.0f;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A) || sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Left))  dir.x -= 1.0f;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D) || sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Right)) dir.x += 1.0f;

        return dir;
    }

    void Player::setPosition(sf::Vector2f pos) {
        position = pos;
    }

    void Player::update(float deltaTime, const WorldMap& worldMap, const sf::RenderWindow& window) {
        sf::Vector2f direction = getInputDirection(window);

        // Normalization (diagonal movement)
        float length = std::sqrt(direction.x * direction.x + direction.y * direction.y);
        if (length > 0.0f) direction /= length;

        // Getting a tile under the player's position
        int16_t tx = static_cast<int16_t>(position.x / GameConfig::TILE_SIZE);
        int16_t ty = static_cast<int16_t>(position.y / GameConfig::TILE_SIZE);

        float terrainModifier = 1.0f;
        if (tx >= 0 && tx < worldMap.getWidth() && ty >= 0 && ty < worldMap.getHeight()) {
            terrainModifier = worldMap.getTileSpeedModifier(tx, ty);
        }

        sf::Vector2f velocity = direction * moveSpeed * terrainModifier * deltaTime;

        // Collision check
        if (!worldMap.isBlockingAtPixel(position.x + velocity.x, position.y)) {
            position.x += velocity.x;
        }

        if (!worldMap.isBlockingAtPixel(position.x, position.y + velocity.y)) {
            position.y += velocity.y;
        }
    }

    void Player::draw(sf::RenderWindow& window, const AssetManager& am) {
        const sf::Texture* tex = am.getSpritesheet("hero"); //  "hero.png" -> "hero"
        float halfSize = GameConfig::TILE_SIZE / 2.0f;
        if (tex) {
            sf::Sprite s(*tex);
            s.setPosition(position);
            s.setOrigin(sf::Vector2f(halfSize, halfSize));
            window.draw(s);
        }
        else {
            float circleCenter = halfSize - 1.0f;
            sf::CircleShape c(circleCenter);
            c.setFillColor(sf::Color::Red);
            c.setPosition(position);
            c.setOrigin(sf::Vector2f(circleCenter, circleCenter));
            window.draw(c);
        }
    }

    std::string Player::getName() const { return name; }

    Vitals Player::getVitals() const {
        return { currentHp, maxHp, currentMp, maxMp, 100.f, 100.f };
    }

    uint8_t Player::getHotbarSize() const { return hotBarSize; }
    uint8_t Player::getInventorySize() const { return inventorySize; }

    std::shared_ptr<IAbility> Player::getHotbarAbility(uint8_t index) const {
        if (index >= 0 && index < hotbar.size()) return hotbar[index];
        return nullptr;
    }

    std::shared_ptr<IAbility> Player::getInventoryItem(uint8_t index) const {
        if (index >= 0 && index < inventory.size()) return inventory[index];
        return nullptr;
    }

    std::shared_ptr<IAbility> Player::getEquipment(EquipSlot slot) const {
        return nullptr;
    }

}
