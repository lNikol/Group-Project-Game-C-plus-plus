#include "GameManager.h"
#include "scenes/WorldScene.h"
#include "scenes/FactionScene.h"
#include "scenes/BattleScene.h"
 #include "combat/AbilityFactory.h"
 #include "combat/UnitFactory.h"
 #include "combat/PropFactory.h"
#include "combat/PartyData.h"
namespace RPG {
    
    GameManager::GameManager()
        : window(sf::VideoMode({ Window::WIDTH, Window::HEIGHT }), Window::TITLE),
        running(false)
    {
        lastWorldPosition = sf::Vector2f(200.0f, 200.0f); 
        activeFaction = FactionID::MainWorld;
        window.setFramerateLimit(Window::BASE_FPS);

        // Assets and structures init
        AssetManager::getInstance().init();
        AssetManager::getInstance().addSpritesheet("player", GameConfig::ANIMATIONS_PATH + "/player.png");
        AssetManager::getInstance().addSpritesheet("npc", GameConfig::ANIMATIONS_PATH + "/npc.png");

        AssetManager::getInstance().addTexture("grass", GameConfig::TEXTURES_PATH + "grass.png");

        // Init start scene
        initGameData();

        AssetManager::getInstance().addSpritesheet("AbilityIcons", GameConfig::TEXTURES_PATH + "placeholders/IconSet.png");
        AssetManager::getInstance().addFont("PixelFont", GameConfig::ASSETS_PATH + "fonts/m5x7.ttf");
        AssetManager::getInstance().addSpritesheet("BattleChars", GameConfig::TEXTURES_PATH + "placeholders/characters1.png");
        AssetManager::getInstance().addSpritesheet("BattleEnemies", GameConfig::TEXTURES_PATH + "placeholders/Monster1.png");
        AssetManager::getInstance().addSpritesheet("BattleProps", GameConfig::TEXTURES_PATH + "placeholders/!Other1.png");
        AssetManager::getInstance().addSpritesheet("BattleTiles", GameConfig::TEXTURES_PATH + "placeholders/Outside_A2.png");
        AssetManager::getInstance().addSpritesheet("BattleBG", GameConfig::TEXTURES_PATH + "placeholders/Mountains3.png");
        m_globalFont = AssetManager::getInstance().getFont("PixelFont");
        m_iconSet = AssetManager::getInstance().getSpritesheet("AbilityIcons");

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
        if (!AbilityFactory::getInstance().loadFromJSON("assets/jsons/abilities.json")) {
            std::cerr << "CRITICAL: Failed to load abilities.json\n";
        }
        if (!UnitFactory::getInstance().loadFromJSON("assets/jsons/units.json")) {
            std::cerr << "CRITICAL: Failed to load units.json\n";
        }
        if (!PropFactory::getInstance().loadFromJSON("assets/jsons/props.json")) {
            std::cerr << "CRITICAL: Failed to load props.json\n";
        }

        auto& party = PartyData::getInstance();
        if (party.activeParty.empty()) {
            auto defaultHero = std::make_shared<CharacterProfile>();
            defaultHero->id = "player_fighter"; // Odwołanie do units.json
            defaultHero->name = "player_fighter"; // Służy również jako klucz do preferredPositions
            defaultHero->currentVitals = { 200.0f, 200.0f, 50.0f, 50.0f, 120.0f, 120.0f };

            party.activeParty.push_back(defaultHero);
        }

        // Shared player initialization
        // player = std::make_shared<Player>(lastWorldPosition);

        // Shared player initialization
        Vitals startStats = { 100.f, 100.f, 50.f, 50.f, 100.f, 100.f }; // HP, MP, Stamina
        playerUnit = std::make_shared<Unit>("Hero", Team::Player, startStats);

        // Create main map
        auto mainMap = getOrLoadMap(activeFaction);
        mainMap->addGameObject(Factory::createPlayer(AssetManager::getInstance(), mainMap.get(), {100, 100}));


        /**
         * @note IMPORTANT
         * If this is a fresh start (no save file), we can add initial NPCs here.
         * In a production system, NPCs would often be part of the .tmj file
         * or a separate "world_state.json" file.
         */

        currentScene = std::make_unique<WorldScene>(*this, mainMap);
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
        auto newMap = std::make_shared<WorldMap>(size.x, size.y);

        // 3. Load Source: Binary Save > Tiled Template > Procedural Generation
        /*if (std::filesystem::exists(savePath)) {
            std::cout << "[GameManager] Loading persistent state for map " << (int)id << "\n";
            newMap->loadFromFile(savePath);
        }*/
        if (id == FactionID::MainWorld) {
            newMap->loadMap(GameConfig::WORLD_PATH + "world.tmj");
        }
        else if (id == FactionID::DarkOrder || id == FactionID::WhiteOrder || id == FactionID::NeutralOrder) {
            newMap->loadMap(GameConfig::WORLD_PATH + "neutral.tmj");
        }
        else {
            std::cout << "[GameManager] Generating new procedural map for faction " << (int)id << "\n";
            newMap->generateObstacles(0.1f);
        }

        //// 4. Memory Management: If too many maps, unload the least recently used
        //if (allMaps.size() >= 5) {
        //    for (auto it = allMaps.begin(); it != allMaps.end(); ) {
        //        // Protect the Main World and the currently active map from being unloaded
        //        if (it->first != FactionID::MainWorld && it->first != activeFaction) {
        //            std::string unloadPath = saveDir + "map_" + std::to_string(static_cast<int>(it->first)) + ".bin";
        //            it->second->saveToFile(unloadPath); // Save changes before purging from RAM
        //            it = allMaps.erase(it);
        //            break;
        //        }
        //        else {
        //            ++it;
        //        }
        //    }
        //}

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
        //auto selectedMap = targetFaction == FactionID::BattleScene ? allMaps[targetFaction] : getOrLoadMap(targetFaction);
        auto selectedMap = getOrLoadMap(targetFaction);
        AssetManager& assetManager = AssetManager::getInstance();

        std::unique_ptr<GameObject> ecsPlayer = nullptr;

        // Extract player from current map
        if (allMaps.count(activeFaction)) {
            ecsPlayer = allMaps[activeFaction]->extractPlayer();

            // Save world position if leaving the main world
            if (ecsPlayer && activeFaction == FactionID::MainWorld && targetFaction != FactionID::MainWorld) {
                lastWorldPosition = ecsPlayer->getPosition();
            }
        }

        // Handle new position
        sf::Vector2f newPosition = { 100.0f, 100.0f };
        if (targetFaction == FactionID::BattleScene) {
            std::cout << "[GameManager] BattleScene: Skipping manual player positioning.\n";
        }
        else if (targetFaction == FactionID::MainWorld) {
            newPosition = lastWorldPosition;
        }
        else if (selectedMap && selectedMap->getHasSpawnPoint()) {
            newPosition = selectedMap->getSpawnPoint();
        }
        else {
            std::cerr << "[GameManager] WARNING: Map has no PlayerSpawn!\n";
        }

        // Inject player into new map
        if (ecsPlayer) {
            ecsPlayer->setPosition(newPosition);
            if (auto* mov = ecsPlayer->getComponent<MovementComponent>()) {
                mov->setWorldMap(selectedMap.get());
            }
            selectedMap->addGameObject(std::move(ecsPlayer));
        }
        else if (selectedMap) {
            selectedMap->addGameObject(Factory::createPlayer(assetManager, selectedMap.get(), newPosition));
            std::cerr << "[GameManager] WARNING: No player in memory! Spawning new one!\n";
        }

        // Change scene
        switch (targetFaction) {
        case FactionID::BattleScene:
            currentScene = std::make_unique<BattleScene>(*this, window, playerUnit, "assets/jsons/encounters/enc_01_tutorial.json");
            break;
        case FactionID::MainWorld:
            currentScene = std::make_unique<WorldScene>(*this, selectedMap);
            break;
        default:
            currentScene = std::make_unique<FactionScene>(*this, selectedMap);
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

            window.setView(sceneView);

            bool uiConsumed = false;
            if (m_hud && m_hud->isMouseOverUI()) {
                uiConsumed = true;
            }
            
            if (const auto* keyPressed = event->getIf<sf::Event::KeyPressed>()) {
                if (keyPressed->scancode == sf::Keyboard::Scancode::Escape) running = false;
                
                if (keyPressed->scancode == sf::Keyboard::Scancode::F3) {
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
    }

    void GameManager::draw() {
        window.clear(sf::Color(0x606030FF));
        
        if (currentScene) currentScene->draw(window);

        window.display();
    }

    GameManager::~GameManager() {
       /* for (auto& [id, map] : allMaps) {
            std::string path = "saves/map_" + std::to_string((int)id) + ".bin";
            map->saveToFile(path);
        }*/
    }
}
