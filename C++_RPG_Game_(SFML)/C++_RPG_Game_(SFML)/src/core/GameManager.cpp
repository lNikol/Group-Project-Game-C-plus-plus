#include "GameManager.h"
#include "scenes/WorldScene.h"
#include "scenes/FactionScene.h"
#include "scenes/BattleScene.h"

namespace RPG {
    
    GameManager::GameManager()
        : window(sf::VideoMode({ Window::WIDTH, Window::HEIGHT }), Window::TITLE),
        running(false)
    {
        lastWorldPosition = sf::Vector2f(200.0f, 200.0f); 
        activeFaction = FactionID::MainWorld;

        window.setFramerateLimit(Window::BASE_FPS);

        // Assets and structures init
        assetManager.init(spritesheetManager);
        spritesheetManager.addSpritesheet(GameConfig::TEXTURES_PATH + "hero.png"); 

        // Init start scene
        initGameData();
        changeScene(FactionID::MainWorld);
    }

    void GameManager::initGameData() {
        // Shared player initialization
        player = std::make_shared<Player>(lastWorldPosition);

        // --- 1. MAIN WORLD ---
        auto mainMap = std::make_shared<WorldMap>(GameConfig::WORLD_WIDTH, GameConfig::WORLD_HEIGHT, assetManager);
        mainMap->generateObstacles(0.07f, player->getPosition(), 3);

        // Add a Leader to MainWorld that sends you to WhiteOrder
        mainMap->addNPC(std::make_unique<NPC>(
            sf::Vector2f(520.0f, 520.0f), "hero", 0, "White Order Envoy", true, FactionID::WhiteOrder
        ));
        mainMap->addNPC(std::make_unique<NPC>(
            sf::Vector2f(300.0f, 420.0f), "hero", 0, "White Orange Order Envoy", false, FactionID::WhiteOrder
        ));

        mainMap->addNPC(std::make_unique<NPC>(
            sf::Vector2f(320.0f, 200.0f), "hero", 0, "White Orange BattleField Envoy", true, FactionID::BattleScene
        ));


        allMaps[FactionID::MainWorld] = mainMap;

        // --- 2. WHITE ORDER BASE (Small, organized) ---
        auto whiteMap = std::make_shared<WorldMap>(25, 25, assetManager);
        whiteMap->generateObstacles(0.02f, sf::Vector2f(400, 520), 2);
        whiteMap->placeStructure(12, 12, StructureType::FactionBase); // Large base in the center
        whiteMap->placeStructure(10, 10, StructureType::Camp);        // Guard camp nearby

        // NPC to go back to MainWorld
        whiteMap->addNPC(std::make_unique<NPC>(
            sf::Vector2f(560.0f, 560.0f), "hero", 1, "White Order Gatekeeper", true, FactionID::MainWorld
        ));
        allMaps[FactionID::WhiteOrder] = whiteMap;

        // --- 3. DARK ORDER BASE (Cramped, dangerous) ---
        auto darkMap = std::make_shared<WorldMap>(15, 15, assetManager);
        // Dark Order has many walls/rocks
        darkMap->generateObstacles(0.15f, sf::Vector2f(400, 480), 2); 

        darkMap->placeStructure(5, 5, StructureType::Wall);
        darkMap->placeStructure(5, 6, StructureType::Wall);
        darkMap->placeStructure(15, 15, StructureType::FactionBase);

        allMaps[FactionID::DarkOrder] = darkMap;

        // --- 4. NEUTRAL ORDER (Wide, empty) ---
        auto neutralMap = std::make_shared<WorldMap>(40, 40, assetManager);
        neutralMap->generateObstacles(0.25f, sf::Vector2f(440, 620), 4);
        neutralMap->placeStructure(5, 5, StructureType::Camp);
        neutralMap->placeStructure(35, 35, StructureType::Camp);

        // Add many trees for "Nature" feel if your generator allows it, 
        // or manually place a few:
        neutralMap->placeStructure(20, 20, StructureType::Tree);
        neutralMap->placeStructure(21, 20, StructureType::Tree);

        allMaps[FactionID::NeutralOrder] = neutralMap;

        std::cout << "GameManager: All faction maps initialized with unique layouts.\n";
    }

    void GameManager::changeScene(FactionID targetFaction) {
        auto selectedMap = allMaps[targetFaction];

        if (activeFaction == FactionID::MainWorld && targetFaction != FactionID::MainWorld) {
            lastWorldPosition = player->getPosition();
        }

        switch (targetFaction) {
        case FactionID::WhiteOrder:
            player->setPosition(sf::Vector2f(400.0f, 520.0f));
            currentScene = std::make_unique<FactionScene>(*this, assetManager, spritesheetManager, selectedMap, player);
            break;

        case FactionID::DarkOrder:
            player->setPosition(sf::Vector2f(400.0f, 480.0f));
            currentScene = std::make_unique<FactionScene>(*this, assetManager, spritesheetManager, selectedMap, player);
            break;

        case FactionID::NeutralOrder:
            player->setPosition(sf::Vector2f(440.0f, 620.0f));
            currentScene = std::make_unique<FactionScene>(*this, assetManager, spritesheetManager, selectedMap, player);
            break;

        case FactionID::BattleScene:
            currentScene = std::make_unique<BattleScene>(*this, selectedMap);
            break;

        default:
        case FactionID::MainWorld:
            player->setPosition(sf::Vector2f(lastWorldPosition));
            currentScene = std::make_unique<WorldScene>(*this, assetManager, spritesheetManager, selectedMap, player);
            break;
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
                
                // For example
                // TODO: change on switch case for special factions and mechanics
                /*if (keyPressed->scancode == sf::Keyboard::Scancode::F) {
                    changeScene(FactionID::WhiteOrder);
                }*/

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