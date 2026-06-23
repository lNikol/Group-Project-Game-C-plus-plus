#pragma once
#include "game_objects/systems/ISystem.h"
#include "game_objects/systems/CollisionSystem.h"
#include "game_objects/components/MovementComponent.h"
#include "game_objects/components/ColliderComponent.h"
#include "game_objects/GameObject.h"
#include "worldmap/WorldMap.h"

namespace RPG {

    class MovementSystem : public ISystem {
        const WorldMap& worldMap;
    public:
        explicit MovementSystem(const WorldMap& map) : worldMap(map) {}

        void update(float dt, std::vector<std::unique_ptr<GameObject>>& entities) override {
            CollisionSystem collision(worldMap);

            for (auto& entity : entities) {
                auto* mov = entity->getComponent<MovementComponent>();
                if (!mov) continue;

                sf::Vector2f vel = mov->getVelocity() * dt;
                if (vel.x == 0.f && vel.y == 0.f) {
                    AudioManager::getInstance().setFootstepsLoop(false);
                    continue;
                }

                AudioManager::getInstance().startFootsteps();

                auto* col = entity->getComponent<ColliderComponent>();
                if (!col) {
                    entity->setPosition(entity->getPosition() + vel);
                    continue;
                }

                sf::FloatRect future = col->getGlobalHitbox();
                sf::Vector2f  finalMove(0.f, 0.f);

                future.position.x += vel.x;
                if (!collision.check(future, entity.get(), entities))
                    finalMove.x = vel.x;
                else
                    future.position.x -= vel.x;

                future.position.y += vel.y;
                if (!collision.check(future, entity.get(), entities))
                    finalMove.y = vel.y;

                entity->setPosition(entity->getPosition() + finalMove);
            }
        }
    };

}
