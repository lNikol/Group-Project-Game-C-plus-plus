#include "CombatAbility.h"
#include "BattleManager.h" // Full definition needed here
#include "combat/Unit.h"
#include <iostream>
#include <cstdlib> // rand()
namespace RPG {

    CombatAbility::CombatAbility(std::string name, int iconIndex, BattleManager& manager, Unit* owner)
        : m_name(name), m_iconIndex(iconIndex), m_manager(manager), m_owner(owner)
    {
    }

    void CombatAbility::setStats(float damage, float range, float manaCost, int cooldownTurns) {
        m_damage = damage;
        m_range = range;
        m_manaCost = manaCost;
        m_maxCooldown = cooldownTurns;
    }

    void CombatAbility::setTargetType(TargetType type) {
        m_targetType = type;
    }

    std::string CombatAbility::getTooltip() const {
        // Build a dynamic string: "Fireball\nDmg: 50\nRange: 150\nMP: 10"
        return m_name + "\n"
            + "Dmg: " + std::to_string((int)m_damage) + "\n"
            + "Range: " + std::to_string((int)m_range) + "\n"
            + "MP: " + std::to_string((int)m_manaCost);
    }

    bool CombatAbility::canBeCast() const {
        if (m_currentCooldown > 0) return false;
        if (m_owner) {
            // Check if owner has enough MP
            return m_owner->getVitals().mp >= m_manaCost;
        }
        return true;
    }

    void CombatAbility::execute() {
        if (!canBeCast()) {
            std::cout << "Cannot cast " << m_name << " (Cooldown or No Mana)!" << std::endl;
            return;
        }

        std::cout << "Selecting target for " << m_name << "..." << std::endl;

        // Handover control to the BattleManager
        m_manager.startTargeting(this);
    }

    void CombatAbility::resolve(std::shared_ptr<Unit> target) {
        if (!m_owner || !target) return;

        // 1. Pay Costs
        if (!m_owner->consumeMana(m_manaCost)) return;
        m_currentCooldown = m_maxCooldown;

        // 2. Calculate Potency & Critical Hit
        // We use absolute value for calculation so crits scale correctly
        float baseAmount = std::abs(m_damage);
        float finalAmount = baseAmount;
        bool isCrit = false;

        float roll = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
        if (roll < m_owner->getCritChance()) {
            isCrit = true;
            finalAmount *= 2.0f;
        }

        // 3. Apply Effect (Heal vs Damage)
        sf::Vector2f spawnPos = target->getRenderPosition();
        bool isHeal = (m_damage < 0); // Convention: Negative damage = Heal

        if (isHeal) {
            // Apply Heal
            target->heal(finalAmount);

            // HEAL STYLE: Green, Floats Up Slowly
            std::string msg = "+" + std::to_string((int)finalAmount);
            if (isCrit) msg += "!";

            // Standard Green for heal, Bright Lime for Crit Heal
            sf::Color healColor = isCrit ? sf::Color(50, 255, 50) : sf::Color(100, 255, 100);

            m_manager.spawnFloatingText(spawnPos, msg, healColor, isCrit ? 22 : 18, { 0.f, -30.f });

            std::cout << m_owner->getName() << " heals " << target->getName() << " for " << finalAmount << std::endl;
        }
        else {
            // Apply Damage
            target->takeDamage(finalAmount);

            // DAMAGE STYLE
            if (isCrit) {
                std::string msg = "CRIT " + std::to_string((int)finalAmount) + "!";
                m_manager.spawnFloatingText(spawnPos, msg, sf::Color(255, 215, 0), 26, { 0.f, -80.f });
            }
            else {
                std::string msg = std::to_string((int)finalAmount);
                m_manager.spawnFloatingText(spawnPos, msg, sf::Color::White, 18, { 0.f, -50.f });
            }
            std::cout << m_owner->getName() << " hits " << target->getName() << " for " << finalAmount << std::endl;
        }

        // 4. Check Death
        if (target->isDead()) {
            m_manager.onUnitDeath(target);
        }
    }

    void CombatAbility::reduceCooldown() {
        if (m_currentCooldown > 0) {
            m_currentCooldown--;
        }
    }
}