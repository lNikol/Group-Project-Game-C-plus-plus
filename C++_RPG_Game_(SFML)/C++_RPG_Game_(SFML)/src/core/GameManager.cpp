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
        assetManager.init();
        assetManager.addSpritesheet("player", GameConfig::ANIMATIONS_PATH + "/player.png");
        assetManager.addSpritesheet("npc", GameConfig::ANIMATIONS_PATH + "/npc.png");;

        // Init start scene
        initGameData();

        assetManager.addSpritesheet("AbilityIcons", GameConfig::TEXTURES_PATH + "placeholders/IconSet.png");
        assetManager.addFont("PixelFont", GameConfig::ASSETS_PATH + "fonts/m5x7.ttf");
        assetManager.addSpritesheet("BattleChars", GameConfig::TEXTURES_PATH + "placeholders/characters1.png");
        assetManager.addSpritesheet("BattleEnemies", GameConfig::TEXTURES_PATH + "placeholders/Monster1.png");
        assetManager.addSpritesheet("BattleProps", GameConfig::TEXTURES_PATH + "placeholders/!Other1.png");
        assetManager.addSpritesheet("BattleTiles", GameConfig::TEXTURES_PATH + "placeholders/Outside_A2.png");
        assetManager.addSpritesheet("BattleBG", GameConfig::TEXTURES_PATH + "placeholders/Mountains3.png");
        m_globalFont = assetManager.getFont("PixelFont");
        m_iconSet = assetManager.getSpritesheet("AbilityIcons");

        /*if (m_iconSet && m_globalFont) {
            m_hud = std::make_unique<BattleHUD>(*m_iconSet, *m_globalFont);
            m_hud->setCombatActor(player.get());
            m_hud->onResize(window.getSize());
        }*/

        //DEBUG
        changeScene(FactionID::MainWorld);
        //changeScene(FactionID::BattleScene);
    }

    void GameManager::initGameData() {

        // Shared player initialization
        player = std::make_shared<Player>(lastWorldPosition, assetManager);

        // Shared player initialization
        Vitals startStats = { 100.f, 100.f, 50.f, 50.f, 100.f, 100.f }; // HP, MP, Stamina
        playerUnit = std::make_shared<Unit>("Hero", Team::Player, startStats);

        Animation npcAnimation = Animation::builder()
            .frameCount(8)
            .frameGap(16)
            .frameStartPos({ 16, 16 })
            .spritesheet(assetManager.getSpritesheet("npc"))
            .looped(true)
            .build();


        auto addNpc = [&](
            uint8_t tileIdx, 
            const std::string& name, 
            bool isLeader, 
            FactionID faction,
            sf::Vector2f pos,
            const std::shared_ptr<WorldMap>& map
        ) {
            auto npc1 = std::make_unique<NPC>(
                tileIdx, name, isLeader, faction
            );
            npc1->setPosition(pos);

            npc1->loadAnimation("idle", npcAnimation);
            npc1->play("idle");


            map->addNPC(std::move(npc1));
        };

        auto mainMap = getOrLoadMap(activeFaction);
        /**
         * @note IMPORTANT
         * If this is a fresh start (no save file), we can add initial NPCs here.
         * In a production system, NPCs would often be part of the .tmj file
         * or a separate "world_state.json" file.
         */

        if (mainMap->getNPCs().empty()) {
            addNpc(0, "White Order Envoy", true, FactionID::WhiteOrder, { 520.f, 520.f }, mainMap);
            addNpc(0, "Dark Order Envoy", true, FactionID::DarkOrder, { 300.0f, 420.0f }, mainMap);
            addNpc(0, "BattleField Envoy", true, FactionID::BattleScene, { 320.0f, 200.0f }, mainMap);
        }

        currentScene = std::make_unique<WorldScene>(*this, mainMap, player);

        std::cout << "[GameManager] Initialized with Lazy Loading. Map: " << (int)activeFaction << "\n";
    }

    std::shared_ptr<WorldMap> GameManager::getOrLoadMap(FactionID id) {
        // 1. Check if already in memory
        if (allMaps.count(id)) return allMaps[id];

        // 2. Determine path for potential binary save
        std::string saveDir = GameConfig::SAVES_PATH;
        std::string savePath = saveDir + "map_" + std::to_string(static_cast<int>(id)) + ".bin";

        if (!std::filesystem::exists(saveDir)) std::filesystem::create_directories(saveDir);

        sf::Vector2u size = getMapSize(id);
        auto newMap = std::make_shared<WorldMap>(size.x, size.y, assetManager);

        // 3. Load Source: Binary Save > Tiled Template > Procedural Generation
        if (std::filesystem::exists(savePath)) {
            std::cout << "[GameManager] Loading persistent state for map " << (int)id << "\n";
            newMap->loadFromFile(savePath);
        }
        else if (id == FactionID::MainWorld) {
            newMap->loadMap(GameConfig::WORLD_PATH + "world.tmj");
        }
        else {
            std::cout << "[GameManager] Generating new procedural map for faction " << (int)id << "\n";
            newMap->generateObstacles(0.1f, { 1000.f, 1000.f }); // Example density and pos
        }

        // 4. Memory Management: If too many maps, unload the least recently used
        if (allMaps.size() >= 5) {
            for (auto it = allMaps.begin(); it != allMaps.end(); ) {
                // Protect the Main World and the currently active map from being unloaded
                if (it->first != FactionID::MainWorld && it->first != activeFaction) {
                    std::string unloadPath = saveDir + "map_" + std::to_string(static_cast<int>(it->first)) + ".bin";
                    it->second->saveToFile(unloadPath); // Save changes before purging from RAM
                    it = allMaps.erase(it);
                    break;
                }
                else {
                    ++it;
                }
            }
        }

        allMaps[id] = newMap;
        return newMap;
    }

    sf::Vector2u GameManager::getMapSize(FactionID id) {
        switch (id) {
        case FactionID::MainWorld:    return { 150, 150 }; 
        case FactionID::WhiteOrder:   return { 40, 40 };   
        case FactionID::DarkOrder:    return { 30, 30 };   
        default:                      return { 50, 50 };
        }
    }


    void GameManager::changeScene(FactionID targetFaction) {
        auto selectedMap = getOrLoadMap(targetFaction);

        if (activeFaction == FactionID::MainWorld && targetFaction != FactionID::MainWorld) {
            lastWorldPosition = player->getPosition();
        }

        switch (targetFaction) {
        case FactionID::WhiteOrder:
            player->setPosition({ 400.0f, 520.0f });
            currentScene = std::make_unique<FactionScene>(*this, selectedMap, player);
            break;

        case FactionID::DarkOrder:
            player->setPosition({ 400.0f, 480.0f });
            currentScene = std::make_unique<FactionScene>(*this, selectedMap, player);
            break;

        case FactionID::NeutralOrder:
            player->setPosition({ 440.0f, 620.0f });
            currentScene = std::make_unique<FactionScene>(*this, selectedMap, player);
            break;

        case FactionID::BattleScene:
            currentScene = std::make_unique<BattleScene>(*this, window, assetManager, playerUnit, selectedMap);
            break;

        default:
        case FactionID::MainWorld:
            player->setPosition(lastWorldPosition);
            currentScene = std::make_unique<WorldScene>(*this, selectedMap, player);
            break;
        }

        activeFaction = targetFaction;
        std::cout << "[GameManager] Scene changed to " << static_cast<int>(targetFaction) << ". Memory cache size: " << allMaps.size() << "\n";
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
            sf::View sceneView = window.getView();
            window.setView(window.getDefaultView());
 
            if (event->is<sf::Event::Closed>()) window.close();
            if (const auto* resized = event->getIf<sf::Event::Resized>()) {
                sf::Vector2f newSize = { (float)resized->size.x, (float)resized->size.y };
                window.setView(sf::View(sf::FloatRect({ 0.f, 0.f }, newSize)));
            }

           /* if (m_hud) {
                m_hud->handleEvent(window, *event);
            }*/
            window.setView(sceneView);

            bool uiConsumed = false;
            if (m_hud && m_hud->isMouseOverUI()) {
                uiConsumed = true;
            }
            
            if (const auto* keyPressed = event->getIf<sf::Event::KeyPressed>()) {
                if (keyPressed->scancode == sf::Keyboard::Scancode::Escape) running = false;
                
                // For example
                // TODO: change on switch case for special factions and mechanics
                /*if (keyPressed->scancode == sf::Keyboard::Scancode::F) {
                    changeScene(FactionID::WhiteOrder);
                }*/
                if (keyPressed->scancode == sf::Keyboard::Scancode::F3) {
                    player->toggleDebugHitbox();
                    for (auto& e : allMaps) {
                        e.second->toggleDebugHitbox();
                    }
                }
            }
            if (!uiConsumed && currentScene) {
                currentScene->handleEvent(window, *event);
            }
        }
    }

    void GameManager::update(float dt) {
        if (currentScene) currentScene->update(dt, window);
        //if (m_hud) m_hud->update(dt);
        //if (m_hud) draw();
    }

    void GameManager::draw() {
        window.clear(sf::Color(0x606030FF));
        
        if (currentScene) currentScene->draw(window, assetManager);

        //if (m_hud) {
        //    window.draw(*m_hud);
        //}

        window.display();
    }

    GameManager::~GameManager() {
        for (auto& [id, map] : allMaps) {
            std::string path = "saves/map_" + std::to_string((int)id) + ".bin";
            map->saveToFile(path);
        }
    }
}
