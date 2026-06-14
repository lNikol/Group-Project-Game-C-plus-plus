#include "WorldMap.h"
#include "core/GameConfig.h"
#include "game_objects/GameObjectFactory.h"
#include "worldmap/AssetManager.h"
#include <iostream>
#include <cmath>
#include <algorithm>

namespace RPG {

    WorldMap::WorldMap(uint32_t w, uint32_t h) : width(w), height(h) {
        initializeGrid();
    }

    void WorldMap::initializeGrid() {
        tiles.assign(width * height, Tile{});
    }

    // ==============================
    // Tiles
    // ==============================

    Tile& WorldMap::at(int32_t x, int32_t y) {
        if (x < 0 || x >= static_cast<int32_t>(width) ||
            y < 0 || y >= static_cast<int32_t>(height)) {
            static Tile dummy;
            return dummy;
        }
        return tiles[y * width + x];
    }

    const Tile& WorldMap::at(int32_t x, int32_t y) const {
        if (x < 0 || x >= static_cast<int32_t>(width) ||
            y < 0 || y >= static_cast<int32_t>(height)) {
            static Tile dummy;
            return dummy;
        }
        return tiles[y * width + x];
    }

    bool WorldMap::isTileBlockingAt(float pixelX, float pixelY) const {
        int32_t tx = static_cast<int32_t>(std::floor(pixelX / GameConfig::TILE_SIZE));
        int32_t ty = static_cast<int32_t>(std::floor(pixelY / GameConfig::TILE_SIZE));
        if (tx < 0 || tx >= static_cast<int32_t>(width) ||
            ty < 0 || ty >= static_cast<int32_t>(height))
            return true; 
        return at(tx, ty).blocksMovement;
    }

    void WorldMap::assignGroundType(uint32_t tx, uint32_t ty, uint32_t gid) {
        if (tx >= width || ty >= height) return;
        std::string name = AssetManager::getInstance().getNameById(gid - 1);
        if (!name.empty())
            at(tx, ty).groundType = AssetManager::getInstance().getDefinition(name).type;
    }

    // ==============================
    // Entities
    // ==============================

    void WorldMap::addGameObject(std::unique_ptr<GameObject> obj) {
        gameObjects.push_back(std::move(obj));
    }

    std::unique_ptr<GameObject> WorldMap::extractPlayer() {
        if (!playerReference) return nullptr;

        auto it = std::find_if(gameObjects.begin(), gameObjects.end(),
            [this](const std::unique_ptr<GameObject>& go) {
                return go.get() == playerReference;
            });

        if (it == gameObjects.end()) return nullptr;

        std::unique_ptr<GameObject> p = std::move(*it);
        gameObjects.erase(it);
        playerReference = nullptr;
        return p;
    }

    void WorldMap::clearAllGameObjects() {
        gameObjects.clear();
        playerReference = nullptr;
    }

    // ==============================
    // Lifecycle
    // ==============================

    void WorldMap::reshape(uint32_t newWidth, uint32_t newHeight, int32_t offX, int32_t offY) {
        width   = newWidth;
        height  = newHeight;
        offsetX = offX;
        offsetY = offY;
        clearAllGameObjects();
        initializeGrid();
    }

    // ==============================
    // Spawning helpers 
    // ==============================

    void WorldMap::spawnNPC(float x, float y, FactionID faction) {
        auto npc = Factory::createNpc(AssetManager::getInstance(), { x, y }, faction);
        addGameObject(std::move(npc));
    }

    void WorldMap::addTriggerZone(float x, float y, float w, float h, int interactionId) {
        auto trigger = std::make_unique<GameObject>("trigger");
        trigger->setPosition(x, y);
        // trigger->addComponent<TriggerComponent>(interactionId, w, h);
        addGameObject(std::move(trigger));
    }

}
