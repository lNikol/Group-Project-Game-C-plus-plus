#pragma once
#include <memory>
#include <SFML/Graphics.hpp>
#include "animation/AnimationLoader.h"
#include "GameObject.h"
#include "worldmap/AssetManager.h"
#include "components/RenderComponent.h"
#include "components/AnimationComponent.h"
#include "components/ColliderComponent.h"
#include "components/MovementComponent.h"
#include "components/PlayerInputComponent.h"
#include "components/AnimationControllerComponent.h"
#include "components/StructureComponent.h"

namespace RPG {
	namespace Factory {

		std::unique_ptr<GameObject> createPlayer(AssetManager& am, sf::Vector2f pos);
		std::unique_ptr<GameObject> createNpc(AssetManager& am, sf::Vector2f pos);
		std::unique_ptr<GameObject> createBox(AssetManager& am, sf::Vector2f pos);

		std::unique_ptr<GameObject> createDynamicObject(AssetManager& am, const std::string& assetName, sf::Vector2f pos);

	}
}