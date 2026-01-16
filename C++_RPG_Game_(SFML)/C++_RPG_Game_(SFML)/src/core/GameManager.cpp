#include "core/GameManager.h"
#include "scenes/WorldScene.h"
#include "scenes/FactionScene.h"

namespace RPG {

    GameManager::GameManager()
        : window(sf::VideoMode({ Window::WIDTH, Window::HEIGHT }), Window::TITLE),
        running(false)
    {
        window.setFramerateLimit(Window::BASE_FPS);

        // Assets and structures init
        assetManager.init(spritesheetManager);
        spritesheetManager.addSpritesheet(GameConfig::TEXTURES_PATH + "hero.png"); 

        // Init start scene
        initGameData();
        changeScene(FactionID::MainWorld);
    }

    void GameManager::initGameData() {
        player = std::make_shared<Player>(200.0f, 200.0f);

        auto mainMap = std::make_shared<WorldMap>(GameConfig::WORLD_WIDTH, GameConfig::WORLD_HEIGHT, assetManager);
        mainMap->generateObstacles(0.07f, player->getPosition(), 3);
        allMaps[FactionID::MainWorld] = mainMap;

        
        // Example of FactionMap, Testing
        auto whiteMap = std::make_shared<WorldMap>(20, 20, assetManager);
        // For future: redMap->addNPC(std::make_unique<NPC>(...));
        allMaps[FactionID::WhiteOrder] = whiteMap;
        whiteMap->placeStructure(8, 8, StructureType::Camp);
        allMaps[FactionID::WhiteOrder] = whiteMap;

    }

    void GameManager::changeScene(FactionID targetFaction) {
        auto selectedMap = allMaps[targetFaction];

        if (targetFaction == FactionID::MainWorld) {
            player->setPosition(sf::Vector2f(200.0f, 200.0f));
            currentScene = std::make_unique<WorldScene>(assetManager, spritesheetManager, selectedMap, player);
        }
        else {
            player->setPosition(sf::Vector2f(400.0f, 320.0f));
            currentScene = std::make_unique<FactionScene>(assetManager, spritesheetManager, selectedMap, player);
        }
    }

    void GameManager::run() {
        running = true;
        sf::Clock clock;

        while (window.isOpen() && running) {
            float dt = clock.restart().asSeconds();

            // Protection against huge dt after Alt-Tab
            if (dt > 0.1f) dt = 0.1f;

            handleEvents();
            update(dt);
            draw();
        }
    }

    void GameManager::handleEvents() {
        while (const std::optional event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>()) window.close();

            if (const auto* keyPressed = event->getIf<sf::Event::KeyPressed>()) {
                if (keyPressed->scancode == sf::Keyboard::Scancode::Escape) running = false;
                
                if (keyPressed->scancode == sf::Keyboard::Scancode::M) {
                    changeScene(FactionID::MainWorld);
                }
                // For example
                // TODO: change on switch case for special factions and mechanics
                if (keyPressed->scancode == sf::Keyboard::Scancode::F) {
                    changeScene(FactionID::WhiteOrder);
                }

                if (currentScene) {
                    currentScene->handleEvents(*event);
                }
            }
        }
    }

    void GameManager::update(float dt) {
        if (currentScene) currentScene->update(dt, window);
    }

    void GameManager::draw() {
        window.clear(sf::Color(0x4B0082FF));
        
        if (currentScene) currentScene->draw(window);

        window.display();
    }
}