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
#include "game_objects/components/GameTriggerHandler.h"
#include "game_objects/components/DialogTriggerHandler.h"
#include "game_objects/systems/SystemManager.h"
#include "game_objects/systems/InputSystem.h"
#include "game_objects/systems/MovementSystem.h"
#include "game_objects/systems/AnimationSystem.h"
#include "game_objects/systems/RenderSystem.h"
#include "game_objects/systems/TriggerSystem.h"
#include "GUI/windows/JournalWindow.h"
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
        // Removed JournalWindow


        float m_questIconJumpTimer = 0.f;
        size_t m_lastActiveQuestCount = 0;

        void checkInteraction();
        virtual void handleInteraction(GameObject* go);

        void initSystems();

    public:
        BaseGameScene(ISceneController& ctrl, std::shared_ptr<WorldMap> wm);
        virtual ~BaseGameScene() = default;
        /**
         * @brief Updates the scene logic.
         * * Handles player movement, camera centering, and updates all NPCs residing
         * on the current map. Can be paused if UI elements (like inventory) are open.
         * * @param dt Delta time since last frame.
         * @param window Reference to the render window.
         */
        void update(float dt, const sf::RenderWindow& window) override;

        /**
         * @brief Renders the game world and entities.
         * * Sets the view to the camera, draws the map layers via WorldMapRenderer,
         * and draws the player and NPCs. Resets view to default for UI rendering.
         * * @param window Reference to the render window.
         */
        void draw(sf::RenderWindow& window) override;
        bool handleEvent(sf::RenderWindow& window, const sf::Event& event) override;
    };

}
