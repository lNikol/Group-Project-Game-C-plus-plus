#include "GameObjectFactory.h"

namespace RPG {
	namespace Factory {

		std::unique_ptr<GameObject> createPlayer(AssetManager& am, sf::Vector2f pos) {

			// Load animations
			const Spritesheet& spritesheet = am.getTest("player");
			auto animations = AnimationLoader::loadAnimations(
				GameConfig::ANIMATIONS_PATH + "player_animations.json",
				&spritesheet
			);

			std::unique_ptr<GameObject> player = std::make_unique<GameObject>();
			player->setPosition(pos);

			// Add sprite
			player->addComponent<RenderComponent>(spritesheet);

			// Add animations
			auto animation = player->addComponent<AnimationComponent>();
			animation->setAnimations(animations);
			animation->play("idle_bottom");

			// Add hitbox
			auto collider = player->addComponent<ColliderComponent>(8, 2, -4, 8);
			collider->setDebug(true);

			// Add movement & player input
			player->addComponent<MovementComponent>(200.f);
			player->addComponent<PlayerInputComponent>();

			// Add animation handling
			player->addComponent<AnimationControllerComponent>();

			return player;
		}

		std::unique_ptr<GameObject> createTestNpc(AssetManager& am, sf::Vector2f pos) {

			const Spritesheet& spritesheet = am.getTest("npc");
			auto animations = AnimationLoader::loadAnimations(
				GameConfig::ANIMATIONS_PATH + "npc_animations.json",
				&spritesheet
			);

			std::unique_ptr<GameObject> npc = std::make_unique<GameObject>();
			npc->setPosition(pos);

			npc->addComponent<RenderComponent>(spritesheet);
			auto animation = npc->addComponent<AnimationComponent>();
			animation->setAnimations(animations);
			animation->play("idle_bottom");

			return npc;
		}

		std::unique_ptr<GameObject> createBox(AssetManager& am, sf::Vector2f pos) {

			const Spritesheet& spritesheet = am.getTest("box");

			std::unique_ptr<GameObject> box = std::make_unique<GameObject>();
			box->setPosition(pos);

			box->addComponent<RenderComponent>(spritesheet);
			auto collider = box->addComponent<ColliderComponent>(32, 32, 0, 16);
			collider->setDebug(true);

			return box;
		}


		std::unique_ptr<GameObject> createDynamicObject(AssetManager& am, const std::string& assetName, sf::Vector2f pos)
		{
			StructureDefinition& def = am.getDefinition(assetName);

			std::unique_ptr<GameObject> go = std::make_unique<GameObject>();
			go->setPosition(pos);

			const sf::Texture* tex = am.getTexture(def.textureKey);
			if (!tex) {
				std::cerr << "[Factory] FATAL: Missing texture '" << def.textureKey << "' for object '" << assetName << "'!\n";
			}
			else {
				go->addComponent<RenderComponent>(*tex, def.textureStartPos, def.size);
			}

			if (def.layer != PlacementLayer::Decoration) {
				go->addComponent<ColliderComponent>(def.hitboxOffset, def.hitboxSize);
			}
			
			go->addComponent<StructureComponent>(&def);

			switch (def.type) {
			case StructureType::Tree:
			case StructureType::Rock:
			case StructureType::Wall:
				break;

			case StructureType::interactObj:
				// TODO: add some interaction components
				break;

			case StructureType::Camp:
				// TODO: add appropriate components
				break;

			default:
				break;
			}

			return go;
		}

	}
}
