#pragma once
#include "Interfaces/IAbility.h"
#include <string>
#include <memory>
#include <functional>

namespace RPG {

    // Forward declaration to avoid circular dependency
    class BattleManager;
    class Unit;
    enum class TargetType {
        Self,
        Enemy,
        Ally,
        Ground
    };

    /**
     * @brief Concrete implementation of IAbility for combat skills (Attacks, Heals).
     * * Implements the "Two-Phase" execution pattern:
     * * 1. execute() -> Switches Game State to Targeting Mode.
     * * 2. resolve() -> Applies the actual effect to the target.
     */
    class CombatAbility : public IAbility {
    public:
        /**
         * @brief Constructs a combat ability.
         * @param name Display name.
         * @param iconIndex Icon index in the spritesheet.
         * @param manager Reference to the BattleManager (to handle state switching).
         * @param owner The Unit that owns this skill (used for checking Mana/Stamina).
         */
        CombatAbility(std::string name, int iconIndex, BattleManager& manager, Unit* owner);

        virtual ~CombatAbility() = default;

        // ==============================
        // Configuration
        // ==============================
        void setStats(float damage, float range, float manaCost, int cooldownTurns);
        void setTargetType(TargetType type);

        float getRange() const { return m_range; }
        TargetType getTargetType() const { return m_targetType; }


        // ==============================
        // IAbility Interface
        // ==============================
        int getIconIndex() const override { return m_iconIndex; }
        std::string getTooltip() const override;

        // Checks if the owner has enough mana/stamina
        bool canBeCast() const override;

        int getCharges() const override { return -1; } // Infinite use
        int getCooldown() const override { return m_currentCooldown; }

        /**
         * @brief Phase 1: Activation
         * * Called by ActionSlot when clicked.
         * * Triggers BattleManager to start targeting.
         */
        void execute() override;


        // ==============================
        // Gameplay Logic
        // ==============================

        /**
         * @brief Phase 2: Resolution
         * * Called by BattleManager when a valid target is clicked.
         * * Applies damage/healing and consumes resources.
         */
        void resolve(std::shared_ptr<Unit> target);

        /**
         * @brief Reduces cooldown (call this at start of turn).
         */
        void reduceCooldown();

    private:
        std::string m_name;
        int m_iconIndex;

        BattleManager& m_manager;
        Unit* m_owner; // weak reference to caster

        // Stats
        float m_damage = 0.f;
        float m_range = 100.f;
        float m_manaCost = 0.f;
        int m_maxCooldown = 0;
        int m_currentCooldown = 0;

        TargetType m_targetType = TargetType::Enemy;
    };
}