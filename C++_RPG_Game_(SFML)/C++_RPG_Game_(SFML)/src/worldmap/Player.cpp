#include "Player.h"

namespace RPG {

    Player::Player(float startX, float startY, const AssetManager& assetManager)
        : name("hero"), CollidingEntity()
    {
        setPosition(startX, startY);
        setHitbox(14.f, 6.f, 8.f);

        hotbar.resize(hotBarSize, nullptr);
        inventory.resize(inventorySize, nullptr);

        const Spritesheet* playerSheet = assetManager.getSpritesheet("player");
        unsigned int animationOffsetY = 64;
        unsigned int runAnimationY = 528;
        unsigned int idleAnimationY = 16;
        unsigned int animationX = 16;

        auto loadAnim = [&](std::string name, sf::Vector2u pos, uint8_t frames) {
            Animation animation = Animation::builder()
                .frameCount(frames)
                .frameStartPos(pos)
                .frameGap(32)
                .spritesheet(playerSheet)
                .build();
            this->loadAnimation(name, animation);
        };

        loadAnim("run_bottom", { animationX, runAnimationY }, 8);
        loadAnim("run_left", { animationX, runAnimationY + animationOffsetY }, 8);
        loadAnim("run_right", { animationX, runAnimationY + animationOffsetY * 2 }, 8);
        loadAnim("run_top", { animationX, runAnimationY + animationOffsetY * 3 }, 8);

        loadAnim("idle_bottom", { animationX, idleAnimationY }, 5);
        loadAnim("idle_left", { animationX, idleAnimationY + animationOffsetY }, 5);
        loadAnim("idle_right", { animationX, idleAnimationY + animationOffsetY * 2 }, 5);
        loadAnim("idle_top", { animationX, idleAnimationY + animationOffsetY * 3 }, 4);

        play("idle_bottom");

        facingDirection = Facing::DOWN;
    }

    Player::Player(sf::Vector2f pos, const AssetManager& assetManager) : 
        Player(pos.x, pos.y, assetManager)
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
        // Input
        sf::Vector2f direction = getInputDirection(window);

        // Normalize
        float length = std::sqrt(direction.x * direction.x + direction.y * direction.y);
        if (length > 0.0f) direction /= length;

        sf::Vector2f velocity = direction * moveSpeed * deltaTime;

        // Move (derived from CollidingEntity)
        moveWithCollision(velocity, worldMap);
        
        // Trigger animations
        if (velocity.x != 0 || velocity.y != 0) {
            if (std::abs(velocity.x) > std::abs(velocity.y)) {
                facingDirection = (velocity.x > 0) ? Facing::RIGHT : Facing::LEFT;
            }
            else {
                facingDirection = (velocity.y > 0) ? Facing::DOWN : Facing::UP;
            }

            // Play Run
            switch (facingDirection) {
                case Facing::UP:    play("run_top"); break;
                case Facing::DOWN:  play("run_bottom"); break;
                case Facing::LEFT:  play("run_left"); break;
                case Facing::RIGHT: play("run_right"); break;
            }
        }
        else {
            // Play Idle based on facingDirection
            switch (facingDirection) {
                case Facing::UP:    play("idle_top"); break;
                case Facing::DOWN:  play("idle_bottom"); break;
                case Facing::LEFT:  play("idle_left"); break;
                case Facing::RIGHT: play("idle_right"); break;
            }
        }

        AnimatedEntity::update(deltaTime);
    }

    void Player::draw(sf::RenderWindow& window) {
        AnimatedEntity::draw(window);
        //CollidingEntity::drawHitbox(window);
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
