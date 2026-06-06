#pragma once
#include <vector>
#include <memory>
#include <SFML/Graphics.hpp>
#include "Tile.h"
#include "game_objects/GameObject.h"
#include "enums.h"

namespace RPG {

    /**
	* @brief Raw spatial and entity data for the game world.
    */
    class WorldMap {
    private:
        uint32_t width;
        uint32_t height;
        int32_t  offsetX = 0;
        int32_t  offsetY = 0;

        sf::Vector2f spawnPoint = { 150.f, 150.f };
        bool hasSpawnPoint = true;

        std::vector<Tile> tiles;
        std::vector<std::unique_ptr<GameObject>> gameObjects;
        GameObject* playerReference = nullptr;

        void initializeGrid();

    public:
        WorldMap(uint32_t w, uint32_t h);

        uint32_t getWidth()   const { return width; }
        uint32_t getHeight()  const { return height; }
        int32_t  getOffsetX() const { return offsetX; }
        int32_t  getOffsetY() const { return offsetY; }
        void     setOffsets(int32_t ox, int32_t oy) { offsetX = ox; offsetY = oy; }

        // Spawn point
        sf::Vector2f getSpawnPoint()    const { return spawnPoint; }
        bool         getHasSpawnPoint() const { return hasSpawnPoint; }
        void         setSpawnPoint(sf::Vector2f sp) { spawnPoint = sp; hasSpawnPoint = true; }
        void         setPlayerInitialPosition(float x, float y) { setSpawnPoint({ x, y }); }

        Tile&       at(int32_t x, int32_t y);
        const Tile& at(int32_t x, int32_t y) const;
        void        assignGroundType(uint32_t tx, uint32_t ty, uint32_t gid);

        /**
         * @brief Is tile blocking at the given pixel position?
         * Raw data — only tile.blocksMovement. No components.
         * Used EXCLUSIVELY by CollisionSystem.
         */
        bool isTileBlockingAt(float pixelX, float pixelY) const;

        // Entity management
        void addGameObject(std::unique_ptr<GameObject> obj); // pure push_back
        void setPlayerReference(GameObject* player) { playerReference = player; }
        void clearPlayerReference()                 { playerReference = nullptr; }

        const GameObject* getPlayer() const { return playerReference; }
        GameObject* getPlayer() { return playerReference; }

        const std::vector<std::unique_ptr<GameObject>>& getGameObjects() const { return gameObjects; }
        std::vector<std::unique_ptr<GameObject>>& getGameObjects() { return gameObjects; }

        std::unique_ptr<GameObject> extractPlayer(); // searches by pointer, not by components  
        void clearAllGameObjects();

        // Lifecycle
        void reshape(uint32_t newWidth, uint32_t newHeight, int32_t offX, int32_t offY);

        // Spawning helpers — called by MapLoader
        void spawnNPC(float x, float y, FactionID faction, int interactionId);
        void addTriggerZone(float x, float y, float w, float h, int interactionId);

        void generateObstacles(float density, uint8_t playerSafeRadius = 2) {} 
    };

}
