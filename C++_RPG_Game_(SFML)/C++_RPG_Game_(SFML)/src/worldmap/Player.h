#pragma once
#include <SFML/Graphics.hpp>
#include <cstdint>
#include <cmath>
#include "core/Constants.h"
#include "WorldMap.h"
#include "interfaces/ICombatActor.h"
#include "entities/CollidingEntity.h"
#include "enums.h"

namespace RPG {

	class Player : public ICombatActor, public CollidingEntity {
	private:
		float moveSpeed = 250.0f; // px/s
		Facing facingDirection;

		std::string name;
		uint8_t hotBarSize = 18;
		uint8_t inventorySize = 20;

		float currentHp = 100.0f;
		float maxHp = 100.0f;
		float currentMp = 50.0f;
		float maxMp = 50.0f;

		bool debugHitbox = false;

		std::vector<std::shared_ptr<IAbility>> hotbar;
		std::vector<std::shared_ptr<IAbility>> inventory;

		GameConfig::Faction faction = GameConfig::Faction::NEUTRAL;

		sf::Vector2f getInputDirection(const sf::RenderWindow& window) const;
	public:
		Player(float startX, float startY, const AssetManager& assetManager);
		Player(sf::Vector2f pos, const AssetManager& assetManager);

		// --- World Logic ---

		/**
		 * @brief Updates player logic and movement
		 * @param deltaTime Time elapsed since last frame
		 * @param worldMap Reference to check collisions and speed modifiers
		 */
		void update(float deltaTime, const WorldMap& worldMap, const sf::RenderWindow& window);
		void draw(sf::RenderWindow& window) override;


		// --- ICombatActor Implementation ---

		void toggleDebugHitbox();

		Vitals getVitals() const override;

		std::string getName() const;

		std::shared_ptr<IAbility> getHotbarAbility(uint8_t index) const override;
		
		uint8_t getHotbarSize() const override;
		uint8_t getInventorySize() const override;

		std::shared_ptr<IAbility> getInventoryItem(uint8_t index) const override;

		std::shared_ptr<IAbility> getEquipment(EquipSlot slot) const override;
	};
}