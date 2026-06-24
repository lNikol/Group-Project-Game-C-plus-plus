#pragma once
#include "game_objects/systems/ISystem.h"
#include "game_objects/components/TriggerComponent.h"
#include "game_objects/components/ITriggerHandler.h"
#include "game_objects/components/ColliderComponent.h"
#include "game_objects/components/PhysicsHelpers.h"
#include "game_objects/GameObject.h"
#include "worldmap/WorldMap.h"
#include <vector>
#include <memory>
#include <cmath>

namespace RPG {

    /**
     * @brief Detects whether the player entered a trigger area and executes handlers.
     *
     * Detection:
     * Circle -> distance between the trigger center and the player's position <= radius
     * Rect   -> player's position is inside the trigger rectangle
     *
     * For Rect, we use the player's feet position (bottom of the hitbox)
     * because the player "stands" on the ground rather than being centered
     * on the sprite.
     *
     * Handlers are executed in order. The first one that returns true
     * stops the chain.
     * Order: GameTriggerHandler -> DialogTriggerHandler -> ...
     *
     * Constructor:
     * TriggerSystem(WorldMap& map, handlers...)
     * handlers are shared_ptr — you can add as many as needed.
    */
    class TriggerSystem : public ISystem {
        const WorldMap& worldMap;
        std::vector<std::shared_ptr<ITriggerHandler>> handlers;

    public:
        TriggerSystem(const WorldMap& map, std::vector<std::shared_ptr<ITriggerHandler>> handlers)
            : worldMap(map), handlers(std::move(handlers)) {}

        void update(float dt, std::vector<std::unique_ptr<GameObject>>& entities) override {
            const GameObject* player = worldMap.getPlayer();
            if (!player) return;

            sf::Vector2f playerFeet = Physics::getFeetPosition(player);

            for (auto& entity : entities) {
                auto* trigger = entity->getComponent<TriggerComponent>();
                if (!trigger || !trigger->active) continue;

                if (!isInside(*trigger, entity->getPosition(), playerFeet)) continue;

                bool handled = false;
                for (auto& handler : handlers) {
                    if (handler->handle(*trigger, entity.get())) {
                        handled = true;
                        break;
                    }
                }

                if (handled && trigger->oneShot) trigger->active = false;
            }
        }

    private:
        bool isInside(const TriggerComponent& trigger, sf::Vector2f triggerPos, sf::Vector2f playerFeet) const {
            switch (trigger.shape) {
            case TriggerShape::Circle: {
                float centerX = triggerPos.x + trigger.radius;
                float centerY = triggerPos.y + trigger.radius;

                float dx = playerFeet.x - centerX;
                float dy = playerFeet.y - centerY;

                return (dx * dx + dy * dy) <= (trigger.radius * trigger.radius);
            }
            case TriggerShape::Rect: {
                return sf::FloatRect({ triggerPos, trigger.rectSize })
                    .contains(playerFeet);
            }
            }
            return false;
        }
    };

}
