#pragma once
#include "Interfaces/IAbility.h"
#include "combat/AbilityData.h"
#include <memory>
#include <vector>

namespace RPG {

    class BattleManager;
    class Unit;

    /**
     * @class CombatAbility
     * @brief Represents a runtime instance of an ability tied to a specific Unit.
     * * This class acts as the bridge between the UI (IAbility), the immutable game
     * data (AbilityDefinition), and the active combat state.
     */
    class CombatAbility : public IAbility {
    public:
        /**
         * @brief Constructs a new Combat Ability instance.
         * @param def A pointer to the shared, read-only definition of this ability.
         * @param manager Reference to the central BattleManager for state changes.
         * @param owner A pointer to the unit that possesses and casts this ability.
         */
        CombatAbility(const AbilityDefinition* def, BattleManager* manager, Unit* owner);
        virtual ~CombatAbility() = default;

        // ==============================
        // IAbility Interface
        // ==============================
        int getIconIndex() const override { return m_def->iconIndex; }
        std::string getTooltip() const override;
        bool canBeCast() const override;
        int getCharges() const override { return m_currentCharges; }
        int getCooldown() const override { return m_currentCooldown; }

        /**
         * @brief Initiates Phase 1 of casting: Targeting.
         * Validates costs and signals the BattleManager to enter Targeting Mode.
         */
        void execute() override;

        // ==============================
        // Gameplay Logic
        // ==============================

        /**
         * @brief Initiates Phase 2 of casting: Resolution (AOE Supported).
         * Deducts resources, triggers cooldowns, and routes targets to the AbilityProcessor.
         * @param targets A list of units selected to receive the ability's effects.
         */
        void resolve(const std::vector<std::shared_ptr<Unit>>& targets);

        /**
         * @brief Decrements the current cooldown timer by 1.
         */
        void reduceCooldown();

        float getRange() const { return m_def->range; }
        float getRadius() const { return m_def->radius; }
        TargetType getTargetType() const { return m_def->targetType; }
        void setOwner(Unit* newOwner) { m_owner = newOwner; }
        void setBattleManager(BattleManager* manager) { m_manager = manager; }
    private:
        const AbilityDefinition* m_def; ///< Pointer to the shared blueprint data.
        
        Unit* m_owner;                  ///< The unit casting this ability.

        int m_currentCooldown = 0;      ///< Turns remaining until the ability can be cast again.
        int m_currentCharges = -1;
    protected:
        BattleManager* m_manager;       ///< Reference to the central game orchestrator.
    };
}