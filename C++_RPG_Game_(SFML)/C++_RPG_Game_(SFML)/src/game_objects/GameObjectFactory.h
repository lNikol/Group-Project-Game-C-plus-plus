#pragma once
#include <memory>
#include <SFML/Graphics.hpp>
#include "animation/AnimationLoader.h"
#include "GameObject.h"
#include "worldmap/AssetManager.h"
#include "components/components.h"

namespace RPG {

	class WorldMap;

	namespace Factory {

		std::unique_ptr<GameObject> createPlayer(AssetManager& am, WorldMap* worldmap, sf::Vector2f pos);
		std::unique_ptr<GameObject> createNpc(AssetManager& am, sf::Vector2f pos, FactionID factionId);
		std::unique_ptr<GameObject> createBox(AssetManager& am, sf::Vector2f pos);

		std::unique_ptr<GameObject> createDynamicObject(AssetManager& am, const std::string& assetName, sf::Vector2f pos);

	}
}