#pragma once
#include "game_objects/components/ColliderComponent.h"
#include "game_objects/GameObject.h"
#include <SFML/Graphics.hpp>

namespace RPG {
namespace Physics {

    /**
     * @brief Returns the entity's global hitbox, or a point-hitbox based on position
     * if no ColliderComponent exists.
     *
     * Used by CollisionSystem (AABB) and TriggerSystem (player position).
     */
    inline sf::FloatRect getHitbox(const GameObject* entity) {
        if (const auto* col = entity->getComponent<ColliderComponent>())
            return col->getGlobalHitbox();
        // Brak ColliaderComponent — zwracamy punkt jako hitbox 0x0
        sf::Vector2f pos = entity->getPosition();
        return sf::FloatRect(pos, { 0.f, 0.f });
    }

    /**
     * @brief Returns the entity's feet position — the center of the bottom edge
     * of the hitbox.
     *
     * Used by TriggerSystem to determine where the player is standing,
     * not where the center of the sprite is.
     */
    inline sf::Vector2f getFeetPosition(const GameObject* entity) {
        sf::FloatRect hb = getHitbox(entity);
        return {
            hb.position.x + hb.size.x / 2.f,
            hb.position.y + hb.size.y
        };
    }

    /**
     * @brief Checks whether two hitboxes intersect (AABB).
     *
     * Wrapper around SFML findIntersection for readability.
     */
    inline bool overlaps(const sf::FloatRect& a, const sf::FloatRect& b) {
        return a.findIntersection(b).has_value();
    }

}}
