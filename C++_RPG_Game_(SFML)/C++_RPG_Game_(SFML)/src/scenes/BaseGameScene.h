#pragma once
#include <memory>
#include <vector>
#include "Scene.h"
#include "ISceneController.h"
#include "worldmap/WorldMap.h"
#include "worldmap/WorldMapRenderer.h"
#include "worldmap/AssetManager.h"
#include "game_objects/components/InteractionComponent.h"
#include "game_objects/components/NpcComponent.h"
#include "game_objects/systems/SystemManager.h"
#include "game_objects/systems/InputSystem.h"
#include "game_objects/systems/MovementSystem.h"
#include "game_objects/systems/AnimationSystem.h"
#include "game_objects/systems/RenderSystem.h"

namespace RPG {

    /**
     * @brief Base class for scenes that display the world map.
     *
     * In the ECS architecture, update() is triggered by the SystemManager, which contains:
     * InputSystem → MovementSystem → AnimationSystem
     *
     * draw() executes the following pipeline:
     * renderer.setupView() → renderer.drawGround() →
     * systemManager.draw() (RenderSystem) → renderer.drawDebugHUD()
     *
     * Derived scenes (e.g., WorldScene, FactionScene) can add their own systems
     * via getSystemManager() or override the draw() method.
     */

    class BaseGameScene : public Scene {
    protected:
        ISceneController& sceneController;
        std::shared_ptr<WorldMap> worldMap;
        WorldMapRenderer worldRenderer;
        SystemManager systemManager;
        bool isInventoryOpen = false;
        std::string sceneName;
        float viewVisibility = 0.5f;

        void checkInteraction();
        virtual void handleInteraction(GameObject* go);

        void initSystems();

    public:
        BaseGameScene(ISceneController& ctrl, std::shared_ptr<WorldMap> wm);
        virtual ~BaseGameScene() = default;

        void update(float dt) override;
        void draw(sf::RenderWindow& window) override;
        bool handleEvent(sf::RenderWindow& window, const sf::Event& event) override;
    };

}
