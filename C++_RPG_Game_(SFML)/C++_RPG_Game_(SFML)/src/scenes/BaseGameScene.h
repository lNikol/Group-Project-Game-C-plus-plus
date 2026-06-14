#pragma once
#include <memory>
#include <vector>
#include "Scene.h"
#include "ISceneController.h"
#include "worldmap/WorldMap.h"
#include "worldmap/WorldMapRenderer.h"
#include "worldmap/Player.h"
#include "worldmap/AssetManager.h"
#include "core/SpritesheetManager.h"
#include "entities/NPC.h"
#include "game_objects/components/InteractionComponent.h"
#include "game_objects/components/NpcComponent.h"


namespace RPG {
    /**
     * @class BaseGameScene
     * @brief Abstract base class for all graphical scenes that display a map.
     * * Handles common functionality like world rendering, player movement,
     * camera tracking, and NPC updates.
     */
    class BaseGameScene : public Scene {
    protected:
        ISceneController& sceneController;
        bool isInventoryOpen = false;
        std::string sceneName;
        // std::unique_ptr<GameUI> gameUI; // Or smth like that

        std::shared_ptr<WorldMap> worldMap;
        //std::shared_ptr<Player> player;
        

        WorldMapRenderer worldRenderer;
        sf::View camera;
        float viewVisibility;


        void checkInteraction();

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

        /**
         * @brief Default event handling for map-based scenes (World, Faction).
         * Can be overridden by BattleScene to disable NPC interactions.
         */
        bool handleEvent(sf::RenderWindow& window, const sf::Event& event) override;

        /**
         * @brief Logic triggered when 'E' is pressed near an NPC.
         */
        virtual void handleInteraction(GameObject* npc);


    };
}