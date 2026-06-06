#pragma once
#include "game_objects/systems/ISystem.h"
#include "game_objects/components/AnimationComponent.h"
#include "game_objects/components/MovementComponent.h"
#include "game_objects/GameObject.h"
#include "worldmap/enums.h"
#include <unordered_map>
#include <cmath>

namespace RPG {

    class AnimationSystem : public ISystem {
        std::unordered_map<GameObject*, Facing> facingMap;
    public:
        void update(float dt, std::vector<std::unique_ptr<GameObject>>& entities) override {
            for (auto& entity : entities) {
                auto* anim = entity->getComponent<AnimationComponent>();
                if (!anim) continue;

                auto* mov = entity->getComponent<MovementComponent>();
                if (mov) {
                    Facing& facing = facingMap[entity.get()];
                    sf::Vector2f vel = mov->getVelocity();
                    bool isMoving = (vel.x != 0.f || vel.y != 0.f);

                    if (isMoving) {
                        if (std::abs(vel.x) > std::abs(vel.y))
                            facing = (vel.x > 0.f) ? Facing::RIGHT : Facing::LEFT;
                        else
                            facing = (vel.y > 0.f) ? Facing::DOWN : Facing::UP;
                    }

                    if (isMoving) {
                        switch (facing) {
                        case Facing::UP:    anim->play("run_top");    break;
                        case Facing::DOWN:  anim->play("run_bottom"); break;
                        case Facing::LEFT:  anim->play("run_left");   break;
                        case Facing::RIGHT: anim->play("run_right");  break;
                        }
                    }
                    else {
                        switch (facing) {
                        case Facing::UP:    anim->play("idle_top");    break;
                        case Facing::DOWN:  anim->play("idle_bottom"); break;
                        case Facing::LEFT:  anim->play("idle_left");   break;
                        case Facing::RIGHT: anim->play("idle_right");  break;
                        }
                    }
                }

                anim->update(dt);
            }
        }
    };
}
