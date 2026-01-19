#include "Player.h"
#include "core/Constants.h"
#include <cmath>

namespace RPG {

    Player::Player(float startX, float startY) : position(startX, startY) {}
    Player::Player(sf::Vector2f pos) : Player(pos.x, pos.y) {}

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

    void Player::draw(sf::RenderWindow& window, const SpritesheetManager& sm) {
        const sf::Texture* tex = sm.getSpritesheet("hero"); //  "hero.png" -> "hero"
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

}
