#pragma once
#include "CombatWorldObject.h"
#include "interfaces/ICombatActor.h"
#include "CombatAbility.h"
#include <vector>
#include <map>
#include <memory> // Added for std::shared_ptr
#include <string>

namespace RPG {

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
        /**
         * @brief Constructs a Unit with specific vitals and team alignment.
         * @param name The display name of the unit.
         * @param team The team (Player, Enemy, Neutral).
         * @param initialVitals Starting HP, MP, Stamina, etc.
         */
        Unit(std::string name, Team team, Vitals initialVitals);

        virtual ~Unit() = default;

        // ==============================
        // Game Logic Methods (State Changers)
        // ==============================

        /**
         * @brief Applies damage to the unit, reducing HP.
         * @param amount The amount of damage to take.
         */
        void takeDamage(float amount);

        /**
         * @brief Restores HP to the unit, capped at MaxHP.
         * @param amount The amount of HP to restore.
         */
        void heal(float amount);

        /**
         * @brief Spends Mana resources.
         * @param amount The amount of Mana to consume.
         * @return true if enough resources existed and were consumed.
         */
        bool consumeMana(float amount);

        /**
         * @brief Spends Stamina resources.
         * @param amount The amount of Stamina to consume.
         * @return true if enough resources existed and were consumed.
         */
        bool consumeStamina(float amount);

        /**
         * @brief Checks if the unit's HP is 0 or less.
         * @return true if dead.
         */
        bool isDead() const;

        /**
         * @brief Gets the unit's team alignment.
         */
        Team getTeam() const;


        // ==============================
        // Inventory & Skill Management
        // ==============================

        /**
         * @brief Adds an ability or item to the unit's inventory.
         */
        void addToInventory(std::shared_ptr<IAbility> item);

        /**
         * @brief Assigns an ability from inventory (or elsewhere) to a specific hotbar slot.
         * @param index The hotbar slot index.
         * @param ability The ability to assign.
         */
        void setHotbarAbility(int index, std::shared_ptr<IAbility> ability);

        /**
         * @brief Equips an item to a specific equipment slot.
         * @param slot The slot to equip to (e.g., Hand_R, Head).
         * @param item The item to equip.
         */
        void equipItem(EquipSlot slot, std::shared_ptr<IAbility> item);

        // ==============================
        // Movement & Update
        // ==============================

        /**
         * @brief Starts moving the unit towards the target coordinate.
         * @param target The logical (x,y) destination.
         */
        void moveTo(sf::Vector2f target);

        /**
         * @brief Processes movement and animation logic.
         * @param dt Delta time (seconds) from the main game loop.
         * @return true if the unit is currently busy (moving/animating).
         */
        bool update(float dt);

        /**
         * @brief Checks if the unit is currently performing a move action.
         */
        bool isMoving() const;

        // ==============================
        // ICombatActor Interface Implementation (For UI)
        // ==============================

        Vitals getVitals() const override;
        std::string getName() const override;

        std::shared_ptr<IAbility> getHotbarAbility(uint8_t index) const override;
        uint8_t getHotbarSize() const override;

        std::shared_ptr<IAbility> getInventoryItem(uint8_t index) const override;
        uint8_t getInventorySize() const override;

        std::shared_ptr<IAbility> getEquipment(EquipSlot slot) const override;

        // ==============================
        // Turn Management
        // ==============================

        void setInitiative(uint8_t val);
        uint8_t getInitiative() const;

        /**
        * @brief Maintenance called automatically when this unit's turn begins.
        * Resets resources and reduces cooldowns.
        */
        void onTurnStart();

    private:
        std::string m_name;
        Team m_team;
        Vitals m_vitals;

        // --- MOVEMENT VARIABLES ---
        uint8_t m_initiative = 10;
        bool m_isMoving = false;
        sf::Vector2f m_targetPosition;
        float m_moveSpeed = 150.0f; // Pixels per second (Logical units)

        /**
         * @brief Updates sprite animation state based on time delta.
         */
        void updateAnimation(float dt);

        // Containers for abilities/items
        std::vector<std::shared_ptr<IAbility>> m_hotbar;
        std::vector<std::shared_ptr<IAbility>> m_inventory;
        std::map<EquipSlot, std::shared_ptr<IAbility>> m_equipment;
    };
}