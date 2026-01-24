#include "CombatAbility.h"
#include "BattleManager.h" // Full definition needed here
#include "combat/Unit.h"
#include <iostream>
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

        // 2. Apply Effect
        std::cout << m_owner->getName() << " casts " << m_name << " on " << target->getName() << "!" << std::endl;

        // Simple damage logic for now
        target->takeDamage(m_damage);

        // 3. Optional: Play Sound/Animation here

        //Check if target dead.
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