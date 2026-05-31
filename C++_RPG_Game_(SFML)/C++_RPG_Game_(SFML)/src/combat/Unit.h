#pragma once
#include "CombatWorldObject.h"
#include "interfaces/ICombatActor.h"
#include "CombatAbility.h"
#include "AbilityData.h" 
#include "StatusEffect.h" 
#include <vector>
#include <map>
#include <memory> 
#include <string>

namespace RPG {

    class BattleManager; // Forward declaration

    enum class Team {
        Player,
        Enemy,
        Neutral
    };

    /**
     * @brief The concrete implementation of a game character.
     * * Inherits WorldObject for Physics (Position, Hitbox).
     * * Inherits ICombatActor for UI Data (Health, Name, Skills).
     */
    class Unit : public CombatWorldObject, public ICombatActor {
    public:
        Unit(std::string name, Team team, Vitals initialVitals);
        virtual ~Unit() = default;

        // ==============================
        // Game Logic Methods
        // ==============================
        void takeDamage(float amount);
        void heal(float amount);
        bool consumeMana(float amount);
        bool consumeStamina(float amount);
        bool isDead() const;
        Team getTeam() const;

        // ==============================
        // Inventory & Skill Management
        // ==============================
        void addToInventory(std::shared_ptr<IAbility> item);
        void setHotbarAbility(int index, std::shared_ptr<IAbility> ability);
        void equipItem(EquipSlot slot, std::shared_ptr<IAbility> item);

        // ==============================
        // Movement & Update
        // ==============================

        /**
         * @brief Starts moving the unit along a calculated A* path.
         * @param path The list of waypoints to follow.
         */
        void setPath(const std::vector<sf::Vector2f>& path);

        /**
         * @brief Processes movement and animation logic.
         */
        bool update(float dt);
        bool isMoving() const;

        // ==============================
        // ICombatActor Interface
        // ==============================
        Vitals getVitals() const override;
        std::string getName() const override;
        /**
         * @brief Overwrites the current vitals (HP, MP, Stamina).
         */
        void setVitals(const Vitals& vitals) { m_vitals = vitals; }
        /**
         * @brief Calculates current stats dynamically (Base Stats + Buffs - Debuffs).
         */
        UnitStats getStats() const override;
        void setBaseStats(const UnitStats& stats) { m_baseStats = stats; }
        std::shared_ptr<IAbility> getHotbarAbility(uint8_t index) const override;
        uint8_t getHotbarSize() const override;
        std::shared_ptr<IAbility> getInventoryItem(uint8_t index) const override;
        uint8_t getInventorySize() const override;
        std::shared_ptr<IAbility> getEquipment(EquipSlot slot) const override;

        // ==============================
        // Turn & Status Effect Management
        // ==============================
        void setInitiative(uint8_t val);
        uint8_t getInitiative() const;

        /**
         * @brief Attaches a lingering effect to this unit.
         * @param effect The calculated ActiveEffect to apply.
         */
        void addStatusEffect(const ActiveEffect& effect);

        /**
         * @brief Checks if the unit currently has a Stun status.
         */
        bool isStunned() const;

        /**
         * @brief Maintenance called automatically when this unit's turn begins.
         * Processes DoTs, HoTs, and ticks down active statuses.
         * @param manager Reference to BattleManager for spawning floating text.
         */
        void onTurnStart(BattleManager& manager);

        void setCritChance(float chance) { m_critChance = chance; }
        float getCritChance() const { return m_critChance; }
        void setDoubleTurnChance(float chance) { m_doubleTurnChance = chance; }
        float getDoubleTurnChance() const { return m_doubleTurnChance; }

    private:
        std::string m_name;
        Team m_team;
        Vitals m_vitals;

        UnitStats m_baseStats;
        std::vector<ActiveEffect> m_activeEffects;

        uint8_t m_initiative = 10;
        bool m_isMoving = false;
        std::vector<sf::Vector2f> m_path; // Replaced m_targetPosition
        size_t m_currentWaypoint = 0;
        float m_moveSpeed = 150.0f;

        void updateAnimation(float dt);

        std::vector<std::shared_ptr<IAbility>> m_hotbar;
        std::vector<std::shared_ptr<IAbility>> m_inventory;
        std::map<EquipSlot, std::shared_ptr<IAbility>> m_equipment;

        float m_critChance = 0.f;
        float m_doubleTurnChance = 0.f;
    };
}