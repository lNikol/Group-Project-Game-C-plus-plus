#pragma once
#include "game_objects/components/ColliderComponent.h"
#include "game_objects/GameObject.h"
#include "worldmap/WorldMap.h"
#include <SFML/Graphics.hpp>
#include <vector>
#include <memory>

namespace RPG {

    /**
     * @brief Collisions in two independent layers.
     *
     * Layer 1 — Tilemap (tile.blocksMovement):
     * Terrain only, no entities — e.g., water, map boundaries.
     * MapLoader does NOT set blocksMovement for structures with a ColliderComponent.
     * Structures (trees, rocks) are handled exclusively by Layer 2.
     *
     * Layer 2 — AABB entity-vs-entity:
     * Precise hitboxes. The only valid collision layer for game objects/entities.
     */
    class CollisionSystem {
        const WorldMap& worldMap;

    public:
        explicit CollisionSystem(const WorldMap& map) : worldMap(map) {}

        bool check(
            const sf::FloatRect& hitbox,
            const GameObject* ignore,
            const std::vector<std::unique_ptr<GameObject>>& entities
        ) const {
            // --- Layer 1: Tilemap ---
            const float r = hitbox.position.x + hitbox.size.x;
            const float b = hitbox.position.y + hitbox.size.y;

            if (worldMap.isTileBlockingAt(hitbox.position.x, hitbox.position.y) ||
                worldMap.isTileBlockingAt(r - 0.1f, hitbox.position.y) ||
                worldMap.isTileBlockingAt(hitbox.position.x, b - 0.1f) ||
                worldMap.isTileBlockingAt(r - 0.1f, b - 0.1f))
                return true;

            // --- Layer 2: AABB entity-vs-entity ---
            for (const auto& entity : entities) {
                if (entity.get() == ignore) continue;
                const auto* col = entity->getComponent<ColliderComponent>();
                if (!col) continue;
                if (hitbox.findIntersection(col->getGlobalHitbox()).has_value())
                    return true;
            }

            return false;
        }
    };

}
