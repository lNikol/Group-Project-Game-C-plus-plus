#pragma once
#include "combat/AbilityData.h"
#include "interfaces/ICombatActor.h"
#include "interfaces/IAbility.h"
#include <string>
#include <vector>
#include <map>
#include <memory>

namespace RPG {
    const int INVENTORY_SIZE = 20;
    /**
     * @brief Persistent data for a single hero.
     * Retains HP/MP between battles and overworld exploration.
     */
    struct CharacterProfile {
        std::string id;
        std::string name;
        Vitals currentVitals;
        UnitStats baseStats;

        std::vector<std::string> unlockedAbilities;
        std::vector<std::shared_ptr<IAbility>> hotbar;
        std::map<EquipSlot, std::shared_ptr<IAbility>> equipment;
    };

    /**
     * @brief Global Singleton managing the player's team and shared inventory.
     */
    class PartyData {
    public:
        static PartyData& getInstance() {
            static PartyData instance;
            return instance;
        }

        std::vector<std::shared_ptr<CharacterProfile>> activeParty;
        std::vector<std::shared_ptr<CharacterProfile>> reserveRoster;

        // slot shared inventory
        std::vector<std::shared_ptr<IAbility>> sharedInventory;
        int gold = 0;
        /**
         * @brief Maps a CharacterProfile ID to their last used logical (x, y) position.
         */
        std::map<std::string, sf::Vector2f> preferredPositions;
        /**
         * @brief Scans the inventory and safely removes any items that have 0 charges.
         */
        void clearConsumedItems() {
            for (auto& item : sharedInventory) {
                if (item && item->getCharges() == 0) {
                    item = nullptr;
                }
            }
        }

        /**
         * @brief Attempts to add an item to the first empty slot.
         */
        bool addItem(std::shared_ptr<IAbility> item) {
            for (auto& slot : sharedInventory) {
                if (slot == nullptr) {
                    slot = item;
                    return true;
                }
            }
            return false; // Inventory full
        }

    private:
        PartyData() {
            // Pre-allocate empty inventory slots
            sharedInventory.resize(INVENTORY_SIZE, nullptr);
        }
    };
}