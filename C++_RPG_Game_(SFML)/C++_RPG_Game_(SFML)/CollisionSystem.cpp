#include "game_objects/systems/CollisionSystem.h"

namespace RPG {

    bool CollisionSystem::check(
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
