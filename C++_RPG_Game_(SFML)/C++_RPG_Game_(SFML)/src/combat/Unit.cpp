#include "Unit.h"
#include "BattleManager.h" 
#include "core/IsoHelpers.h"
#include <cmath>
#include <iostream> 

namespace RPG {

    Unit::Unit(std::string name, Team team, Vitals initialVitals)
        : m_name(name), m_team(team), m_vitals(initialVitals)
    {
        setColliderSize(20.0f, 20.0f);
    }

    void Unit::takeDamage(float amount) {
        m_vitals.hp -= amount;
        if (m_vitals.hp < 0) m_vitals.hp = 0;
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

    void Unit::addToInventory(std::shared_ptr<IAbility> item) {
        m_inventory.push_back(item);
    }

    void Unit::setHotbarAbility(int index, std::shared_ptr<IAbility> ability) {
        if (index >= 0) {
            if (static_cast<size_t>(index) >= m_hotbar.size()) {
                m_hotbar.resize(index + 1);
            }
            m_hotbar[index] = ability;
        }
    }

    void Unit::equipItem(EquipSlot slot, std::shared_ptr<IAbility> item) {
        m_equipment[slot] = item;
    }

    void Unit::setPath(const std::vector<sf::Vector2f>& path) {
        if (path.empty()) return;
        m_path = path;
        m_currentWaypoint = 0;
        m_isMoving = true;
    }

    bool Unit::update(float dt) {
        if (!m_isMoving || m_path.empty()) {
            updateAnimation(dt);
            return false;
        }

        sf::Vector2f currentPos = getLogicalPosition();
        sf::Vector2f targetPos = m_path[m_currentWaypoint];

        sf::Vector2f diff = targetPos - currentPos;
        float distance = std::sqrt(diff.x * diff.x + diff.y * diff.y);

        if (distance < 2.0f) {
            setLogicalPosition(targetPos.x, targetPos.y);
            m_currentWaypoint++;

            if (m_currentWaypoint >= m_path.size()) {
                m_isMoving = false;
                m_path.clear();
                return false;
            }
            return true;
        }

        sf::Vector2f direction = diff / distance;
        sf::Vector2f moveStep = direction * m_moveSpeed * dt;

        setLogicalPosition(currentPos.x + moveStep.x, currentPos.y + moveStep.y);

        if (m_sprite.has_value()) {
            if (moveStep.x < -0.1f) m_sprite->setScale({ -2.f, 2.f });
            else if (moveStep.x > 0.1f) m_sprite->setScale({ 2.f, 2.f });
        }

        updateAnimation(dt);
        return true;
    }

    bool Unit::isMoving() const {
        return m_isMoving;
    }

    void Unit::updateAnimation(float dt) {}

    Vitals Unit::getVitals() const { return m_vitals; }
    std::string Unit::getName() const { return m_name; }

    UnitStats Unit::getStats() const {
        UnitStats current = m_baseStats;
        for (const auto& status : m_activeEffects) {
            if (status.baseEffect.type == EffectType::Buff && status.baseEffect.targetStat != StatType::None) {
                current[status.baseEffect.targetStat] += status.lockedPower;
            }
            else if (status.baseEffect.type == EffectType::Debuff && status.baseEffect.targetStat != StatType::None) {
                current[status.baseEffect.targetStat] -= status.lockedPower;
            }
        }
        return current;
    }

    std::shared_ptr<IAbility> Unit::getHotbarAbility(uint8_t index) const {
        if (index >= 0 && index < m_hotbar.size()) return m_hotbar[index];
        return nullptr;
    }

    uint8_t Unit::getHotbarSize() const { return static_cast<uint8_t>(m_hotbar.size()); }

    std::shared_ptr<IAbility> Unit::getInventoryItem(uint8_t index) const {
        if (index >= 0 && index < m_inventory.size()) return m_inventory[index];
        return nullptr;
    }

    uint8_t Unit::getInventorySize() const { return static_cast<uint8_t>(m_inventory.size()); }

    std::shared_ptr<IAbility> Unit::getEquipment(EquipSlot slot) const {
        auto it = m_equipment.find(slot);
        if (it != m_equipment.end()) return it->second;
        return nullptr;
    }

    void Unit::setInitiative(uint8_t val) { m_initiative = val; }
    uint8_t Unit::getInitiative() const { return m_initiative; }

    void Unit::addStatusEffect(const ActiveEffect& effect) {
        m_activeEffects.push_back(effect);
    }

    bool Unit::isStunned() const {
        for (const auto& effect : m_activeEffects) {
            if (effect.baseEffect.type == EffectType::Stun) return true;
        }
        return false;
    }

    void Unit::onTurnStart(BattleManager& manager) {
        for (auto& ability : m_hotbar) {
            if (ability) {
                if (auto combatSkill = std::dynamic_pointer_cast<CombatAbility>(ability)) {
                    combatSkill->reduceCooldown();
                }
            }
        }

        m_vitals.stamina = m_vitals.maxStamina;
        m_vitals.mp = m_vitals.maxMp;

        for (auto it = m_activeEffects.begin(); it != m_activeEffects.end(); ) {
            if (it->baseEffect.type == EffectType::Damage) {
                takeDamage(it->lockedPower);
                manager.spawnFloatingText(getRenderPosition(), std::to_string(static_cast<int>(it->lockedPower)), sf::Color(150, 0, 200), 18);
            }
            else if (it->baseEffect.type == EffectType::Heal) {
                heal(it->lockedPower);
                manager.spawnFloatingText(getRenderPosition(), "+" + std::to_string(static_cast<int>(it->lockedPower)), sf::Color(50, 255, 100), 18);
            }

            it->turnsRemaining--;
            if (it->turnsRemaining <= 0) it = m_activeEffects.erase(it);
            else ++it;
        }

        std::cout << m_name << " starts their turn." << std::endl;
    }
}