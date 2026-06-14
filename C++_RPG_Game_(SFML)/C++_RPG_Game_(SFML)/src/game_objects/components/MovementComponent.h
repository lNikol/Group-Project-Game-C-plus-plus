#pragma once
#include "Component.h"
#include "combat/AudioManager.h"

namespace RPG {

    class WorldMap;

    /**
     * @brief Movement data for an entity. Logic is handled in MovementSystem.
     * update() is intentionally absent.
     */
    class MovementComponent : public Component {
        float        speed;
        sf::Vector2f direction;
        WorldMap*    worldmap;

    public:
        MovementComponent(WorldMap* worldmap, float speed)
            : speed(speed), direction(0.f, 0.f), worldmap(worldmap) {}

        void            setDirection(sf::Vector2f dir) { direction = dir; }
        sf::Vector2f    getVelocity()    const { return direction * speed; }
        float           getSpeed()       const { return speed; }
        void            setSpeed(float s)      { speed = s; }
        const WorldMap* getWorldMap()    const { return worldmap; }
        void            setWorldMap(WorldMap* wm) { worldmap = wm; }
    };

}
