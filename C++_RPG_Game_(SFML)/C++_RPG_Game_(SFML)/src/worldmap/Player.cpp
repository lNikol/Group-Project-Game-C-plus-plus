#include "Player.h"
#include "core/Constants.h"
#include <cmath>

namespace RPG {

    Player::Player(float startX, float startY, std::string name) 
        : name(name)
    {
        setPosition(startX, startY);
        hotbar.resize(hotBarSize, nullptr);
        inventory.resize(inventorySize, nullptr);
    }
    Player::Player(sf::Vector2f pos, std::string name) : 
        Player(pos.x, pos.y, name)
    {}

    sf::Vector2f Player::getInputDirection(const sf::RenderWindow& window) const {

        sf::Vector2f dir(0.0f, 0.0f);

        if (!window.hasFocus()) return dir;

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W) || sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Up))    dir.y -= 1.0f;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S) || sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Down))  dir.y += 1.0f;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A) || sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Left))  dir.x -= 1.0f;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D) || sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Right)) dir.x += 1.0f;

        return dir;
    }


    void Player::update(float deltaTime, const WorldMap& worldMap, const sf::RenderWindow& window) {
        sf::Vector2f direction = getInputDirection(window);

        // --- 1. COORDINATE CONVERSION ---
        // Since Origin is Center, getPosition() returns the Center.
        sf::Vector2f centerPos = getPosition();
        sf::Vector2f origin = sprite.getOrigin();

        // Calculate the actual Top-Left for hitbox logic
        sf::Vector2f topLeft = centerPos - origin;

        // --- 2. DEFINE HITBOX ---
        sf::FloatRect spriteBounds = sprite.getLocalBounds();
        float marginX = 4.0f;
        float marginY = 4.0f;

        sf::FloatRect hitbox;
        // Hitbox is relative to the calculated Top-Left
        hitbox.position.x = topLeft.x + marginX;
        hitbox.position.y = topLeft.y + marginY;

        hitbox.size.x = spriteBounds.size.x - (marginX * 2);
        hitbox.size.y = spriteBounds.size.y - (marginY * 2);

        // --- 3. MOVEMENT MATH ---
        // ... normalization ...
        float length = std::sqrt(direction.x * direction.x + direction.y * direction.y);
        if (length > 0.0f) direction /= length;

        sf::Vector2f velocity = direction * moveSpeed * deltaTime;

        // --- 4. COLLISION CHECKS ---
        // (Logic stays mostly the same, but we update 'topLeft' instead of 'currentPos')

        // X AXIS
        float nextHitboxX = hitbox.position.x + velocity.x;
        bool collisionX = false;

        // Use your existing checking logic...
        if (velocity.x > 0) { /* Check Right */
            if (worldMap.isBlockingAtPixel(nextHitboxX + hitbox.size.x, hitbox.position.y) ||
                worldMap.isBlockingAtPixel(nextHitboxX + hitbox.size.x, hitbox.position.y + hitbox.size.y))
                collisionX = true;
        }
        else if (velocity.x < 0) { /* Check Left */
            if (worldMap.isBlockingAtPixel(nextHitboxX, hitbox.position.y) ||
                worldMap.isBlockingAtPixel(nextHitboxX, hitbox.position.y + hitbox.size.y))
                collisionX = true;
        }

        if (!collisionX) {
            topLeft.x += velocity.x;        // Move our reference Top-Left
            hitbox.position.x += velocity.x; // Update hitbox for Y check
        }

        // Y AXIS
        float nextHitboxY = hitbox.position.y + velocity.y;
        bool collisionY = false;

        // Use your existing checking logic...
        if (velocity.y > 0) { /* Check Down */
            if (worldMap.isBlockingAtPixel(hitbox.position.x, nextHitboxY + hitbox.size.y) ||
                worldMap.isBlockingAtPixel(hitbox.position.x + hitbox.size.x, nextHitboxY + hitbox.size.y))
                collisionY = true;
        }
        else if (velocity.y < 0) { /* Check Up */
            if (worldMap.isBlockingAtPixel(hitbox.position.x, nextHitboxY) ||
                worldMap.isBlockingAtPixel(hitbox.position.x + hitbox.size.x, nextHitboxY))
                collisionY = true;
        }

        if (!collisionY) {
            topLeft.y += velocity.y;
        }

        // --- 5. ANIMATION & FLIP ---
        // (Your existing flip logic works perfectly now because Origin is Center!)
        if (velocity.x != 0.0f || velocity.y != 0.0f) {
            play("player_running");
            if (velocity.x < 0) sprite.setScale({ -1.0f, 1.0f });
            else if (velocity.x > 0) sprite.setScale({ 1.0f, 1.0f });
        }
        else {
            play("player_idle");
        }

        // --- 6. FINALIZE ---
        AnimatedEntity::update(deltaTime);

        // CRITICAL: Convert Top-Left back to Center before setting sprite position
        setPosition(topLeft + origin);
    }

    void Player::draw(sf::RenderWindow& window, const AssetManager& am) {
        AnimatedEntity::draw(window);
        /*
        const sf::Texture* tex = am.getSpritesheet("player"); //  "hero.png" -> "hero"
        float halfSize = GameConfig::TILE_SIZE / 2.0f;
        if (tex) {
            sf::Sprite s(*tex);
            s.setPosition(position);
            s.setOrigin(sf::Vector2f(halfSize, halfSize));
            s.setScale({2.0, 2.0});
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
        */
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
