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

namespace RPG {
	namespace Factory {

		std::shared_ptr<GameObject> createPlayer(AssetManager& am, sf::Vector2f pos);
		std::unique_ptr<GameObject> createTestNpc(AssetManager& am, sf::Vector2f pos);
		std::unique_ptr<GameObject> createBox(AssetManager& am, sf::Vector2f pos);

	}
}