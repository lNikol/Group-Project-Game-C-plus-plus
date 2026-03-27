#include "CombatAbility.h"
#include "AbilityProcessor.h"
#include "combat/Unit.h"
#include "BattleManager.h"
#include <sstream>

namespace RPG {

    CombatAbility::CombatAbility(const AbilityDefinition* def, BattleManager& manager, Unit* owner)
        : m_def(def), m_manager(manager), m_owner(owner) {
    }

    std::string CombatAbility::getTooltip() const {
        std::stringstream ss;
        ss << m_def->name << "\n" << m_def->description << "\n";
        if (m_def->manaCost > 0) ss << "MP: " << m_def->manaCost << "\n";
        if (m_def->staminaCost > 0) ss << "STA: " << m_def->staminaCost << "\n";
        if (m_def->cooldownTurns > 0) ss << "CD: " << m_def->cooldownTurns << " Turns";
        return ss.str();
    }

    bool CombatAbility::canBeCast() const {
        if (m_currentCooldown > 0) return false;
        if (m_owner) {
            if (m_owner->getVitals().mp < m_def->manaCost) return false;
            if (m_owner->getVitals().stamina < m_def->staminaCost) return false;
        }
        return true;
    }

    void CombatAbility::execute() {
        if (!canBeCast()) return;
        m_manager.startTargeting(this);
    }

    void CombatAbility::resolve(const std::vector<std::shared_ptr<Unit>>& targets) {
        if (!m_owner) return;

        // 1. Pay Resource Costs (Only once per cast!)
        if (m_def->manaCost > 0) m_owner->consumeMana(m_def->manaCost);
        if (m_def->staminaCost > 0) m_owner->consumeStamina(m_def->staminaCost);

        // 2. Start Cooldown
        m_currentCooldown = m_def->cooldownTurns;

        // 3. Process All Effects for ALL Targets
        for (auto& target : targets) {
            if (target) {
                AbilityProcessor::processAbility(*m_def, *m_owner, *target, m_manager);
            }
        }
    }

    void CombatAbility::reduceCooldown() {
        if (m_currentCooldown > 0) {
            m_currentCooldown--;
        }
    }
}