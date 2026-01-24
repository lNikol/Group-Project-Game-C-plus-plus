#include "Unit.h"
#include "core/IsoHelpers.h"
#include <cmath>
#include <iostream> 

namespace RPG {

    Unit::Unit(std::string name, Team team, Vitals initialVitals)
        : m_name(name), m_team(team), m_vitals(initialVitals)
    {
        // Default collider size for a human-sized unit (logical units)
        setColliderSize(20.0f, 20.0f);
    }

    // ==============================
    // Game Logic Methods
    // ==============================

    void Unit::takeDamage(float amount) {
        m_vitals.hp -= amount;
        if (m_vitals.hp < 0) m_vitals.hp = 0;

        // Debug output
        std::cout << m_name << " took " << amount << " dmg. HP: "
            << m_vitals.hp << "/" << m_vitals.maxHp << std::endl;
    }

    void Unit::heal(float amount) {
        m_vitals.hp += amount;
        if (m_vitals.hp > m_vitals.maxHp) m_vitals.hp = m_vitals.maxHp;
    }

    bool Unit::consumeMana(float amount) {
        if (m_vitals.mp >= amount) {
            m_vitals.mp -= amount;
            return true;
        }
        return false;
    }

    bool Unit::consumeStamina(float amount) {
        if (m_vitals.stamina >= amount) {
            m_vitals.stamina -= amount;
            return true;
        }
        return false;
    }

    bool Unit::isDead() const {
        return m_vitals.hp <= 0;
    }

    Team Unit::getTeam() const {
        return m_team;
    }

    // ==============================
    // Inventory & Skill Management
    // ==============================

    void Unit::addToInventory(std::shared_ptr<IAbility> item) {
        m_inventory.push_back(item);
    }

    void Unit::setHotbarAbility(int index, std::shared_ptr<IAbility> ability) {
        if (index >= 0) {
            // Ensure vector is large enough
            if (static_cast<size_t>(index) >= m_hotbar.size()) {
                m_hotbar.resize(index + 1);
            }
            m_hotbar[index] = ability;
        }
    }

    void Unit::equipItem(EquipSlot slot, std::shared_ptr<IAbility> item) {
        m_equipment[slot] = item;
    }

    // ==============================
    // Movement & Update
    // ==============================

    void Unit::moveTo(sf::Vector2f target) {
        m_targetPosition = target;
        m_isMoving = true;
    }

    bool Unit::update(float dt) {
        if (!m_isMoving) {
            // If we are idle, maybe play "Idle" animation here?
            // updateAnimation(dt); 
            return false;
        }

        // 1. Calculate Direction
        sf::Vector2f currentPos = getLogicalPosition();
        sf::Vector2f diff = m_targetPosition - currentPos;
        float distance = std::sqrt(diff.x * diff.x + diff.y * diff.y);

        // 2. Check Arrival (Threshold)
        if (distance < 2.0f) {
            // Snap to exact target and stop
            setLogicalPosition(m_targetPosition.x, m_targetPosition.y);
            m_isMoving = false;
            return false; // Not busy anymore
        }

        // 3. Move
        sf::Vector2f direction = diff / distance; // Normalize
        sf::Vector2f moveStep = direction * m_moveSpeed * dt;

        setLogicalPosition(currentPos.x + moveStep.x, currentPos.y + moveStep.y);

        // 4. Handle Visuals (Facing Direction)
        if (m_sprite.has_value()) {
            // If moving Left (negative X), flip sprite
            if (moveStep.x < -0.1f) m_sprite->setScale({ -2.f, 2.f });
            // If moving Right (positive X), normal sprite
            else if (moveStep.x > 0.1f) m_sprite->setScale({ 2.f, 2.f });
        }

        // 5. Update Animation State
        updateAnimation(dt);

        return true; // Still busy moving
    }

    bool Unit::isMoving() const {
        return m_isMoving;
    }

    void Unit::updateAnimation(float dt) {
        // SPACE FOR ANIMATION LOGIC LATER
        // Example:
        // m_animTimer += dt;
        // if (m_animTimer > 0.1f) {
        //     currentFrame++;
        //     if (currentFrame > maxFrames) currentFrame = 0;
        //     m_sprite->setTextureRect(...);
        // }
    }

    // ==============================
    // ICombatActor Interface Implementation
    // ==============================

    Vitals Unit::getVitals() const {
        return m_vitals;
    }

    std::string Unit::getName() const {
        return m_name;
    }

    std::shared_ptr<IAbility> Unit::getHotbarAbility(uint8_t index) const {
        if (index >= 0 && index < m_hotbar.size()) {
            return m_hotbar[index];
        }
        return nullptr;
    }

    uint8_t Unit::getHotbarSize() const {
        return static_cast<uint8_t>(m_hotbar.size());
    }

    std::shared_ptr<IAbility> Unit::getInventoryItem(uint8_t index) const {
        if (index >= 0 && index < m_inventory.size()) {
            return m_inventory[index];
        }
        return nullptr;
    }

    uint8_t Unit::getInventorySize() const {
        return static_cast<uint8_t>(m_inventory.size());
    }

    std::shared_ptr<IAbility> Unit::getEquipment(EquipSlot slot) const {
        auto it = m_equipment.find(slot);
        if (it != m_equipment.end()) {
            return it->second;
        }
        return nullptr;
    }

    // ==============================
    // Turn Management
    // ==============================

    void Unit::setInitiative(uint8_t val) {
        m_initiative = val;
    }

    uint8_t Unit::getInitiative() const {
        return m_initiative;
    }

    void Unit::onTurnStart() {
        // 1. Reduce Cooldowns on all abilities
        for (auto& ability : m_hotbar) {
            if (ability) {
                if (auto combatSkill = std::dynamic_pointer_cast<CombatAbility>(ability)) {
                    combatSkill->reduceCooldown();
                }
            }
        }

        // 2. Regen
        m_vitals.stamina = m_vitals.maxStamina;
        m_vitals.mp = m_vitals.maxMp;

        std::cout << m_name << " starts their turn." << std::endl;
    }

}