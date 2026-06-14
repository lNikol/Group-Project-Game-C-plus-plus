#include "GameManager.h"
#include "scenes/WorldScene.h"
#include "scenes/FactionScene.h"
#include "scenes/BattleScene.h"
 #include "combat/AbilityFactory.h"
 #include "combat/UnitFactory.h"
 #include "combat/PropFactory.h"
#include "combat/PartyData.h"
#include "worldmap/MapLoader.h"

namespace RPG {

    GameManager::GameManager()
        : window(sf::VideoMode({ Window::WIDTH, Window::HEIGHT }), Window::TITLE),
          running(false)
    {
        lastWorldPosition = sf::Vector2f(-1.0f, -1.0f);
        activeFaction     = FactionID::MainWorld;
        window.setFramerateLimit(Window::BASE_FPS);

        AssetManager::getInstance().init();
        AssetManager::getInstance().addSpritesheet("player", GameConfig::ANIMATIONS_PATH + "player.png");
        AssetManager::getInstance().addSpritesheet("npc",    GameConfig::ANIMATIONS_PATH + "npc.png");
        AssetManager::getInstance().addTexture("grass",      GameConfig::TEXTURES_PATH   + "grass.png");

        initGameData();

        AssetManager::getInstance().addSpritesheet("AbilityIcons",  GameConfig::TEXTURES_PATH + "placeholders/IconSet.png");
        AssetManager::getInstance().addFont("PixelFont",            GameConfig::ASSETS_PATH   + "fonts/m5x7.ttf");
        AssetManager::getInstance().addSpritesheet("BattleChars",   GameConfig::TEXTURES_PATH + "placeholders/characters1.png");
        AssetManager::getInstance().addSpritesheet("BattleEnemies", GameConfig::TEXTURES_PATH + "placeholders/Monster1.png");
        AssetManager::getInstance().addSpritesheet("BattleProps",   GameConfig::TEXTURES_PATH + "placeholders/!Other1.png");
        AssetManager::getInstance().addSpritesheet("BattleTiles",   GameConfig::TEXTURES_PATH + "placeholders/Outside_A2.png");
        AssetManager::getInstance().addSpritesheet("BattleBG",      GameConfig::TEXTURES_PATH + "placeholders/Mountains3.png");

        m_globalFont = AssetManager::getInstance().getFont("PixelFont");
        m_iconSet    = AssetManager::getInstance().getSpritesheet("AbilityIcons");

        changeScene(FactionID::MainWorld); 
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

        auto mainMap = getOrLoadMap(activeFaction);

        auto player = Factory::createPlayer(AssetManager::getInstance(), mainMap.get(), mainMap->getSpawnPoint());
        GameObject* playerPtr = player.get();
        mainMap->addGameObject(std::move(player));
        mainMap->setPlayerReference(playerPtr);

        currentScene = std::make_unique<WorldScene>(*this, mainMap);
        std::cout << "[GameManager] Init done. Map: " << (int)activeFaction << "\n";
    }

    std::shared_ptr<WorldMap> GameManager::getOrLoadMap(FactionID id) {
        if (allMaps.count(id)) return allMaps[id];

        sf::Vector2u size = getMapSize(id);
        auto map = std::make_shared<WorldMap>(size.x, size.y);

        std::string path;
        switch (id) {
            case FactionID::MainWorld:    path = GameConfig::WORLD_PATH + "world.tmj";       break;
            case FactionID::WhiteOrder:   path = GameConfig::WORLD_PATH + "white_base.tmj";  break;
            case FactionID::DarkOrder:    path = GameConfig::WORLD_PATH + "dark_base.tmj";   break;
            case FactionID::NeutralOrder: path = GameConfig::WORLD_PATH + "neutral.tmj";     break;
            default:                      path = GameConfig::WORLD_PATH + "neutral.tmj";     break;
        }

        std::cout << "[GameManager] Loading: " << path << "\n";
        MapLoader::loadFromTiled(path, *map);

        allMaps[id] = map;
        return map;
    }

    sf::Vector2u GameManager::getMapSize(FactionID id) {
        switch (id) {
            case FactionID::MainWorld:  return { 150, 150 };
            case FactionID::WhiteOrder: return { 40,  40  };
            case FactionID::DarkOrder:  return { 30,  30  };
            default:                    return { 50,  50  };
        }
    }

    void GameManager::changeScene(FactionID targetFaction) {
        auto selectedMap = getOrLoadMap(targetFaction);

        std::unique_ptr<GameObject> player = nullptr;
        if (allMaps.count(activeFaction)) {
            player = allMaps[activeFaction]->extractPlayer();
            if (player && activeFaction == FactionID::MainWorld && targetFaction != FactionID::MainWorld)
                lastWorldPosition = player->getPosition();
        }

        sf::Vector2f newPos = { 100.f, 100.f };
        if (targetFaction == FactionID::BattleScene) {
            //
        }
        else if (targetFaction == FactionID::MainWorld && lastWorldPosition.x >= 0.f) {
            newPos = lastWorldPosition;
        }
        else if (selectedMap->getHasSpawnPoint()) {
            newPos = selectedMap->getSpawnPoint();
        }
        else {
            std::cerr << "[GameManager] WARNING: No spawn point on map!\n";
        }

        // Inject player into the new map and explicitly register
        if (player) {
            player->setPosition(newPos);
            if (auto* mov = player->getComponent<MovementComponent>())
                mov->setWorldMap(selectedMap.get());

            GameObject* ptr = player.get();
            selectedMap->addGameObject(std::move(player));
            selectedMap->setPlayerReference(ptr); 
        }
        else {
            std::cerr << "[GameManager] WARNING: No player — spawning new one.\n";
            auto newPlayer = Factory::createPlayer(AssetManager::getInstance(), selectedMap.get(), newPos);
            GameObject* ptr = newPlayer.get();
            selectedMap->addGameObject(std::move(newPlayer));
            selectedMap->setPlayerReference(ptr); 
        }

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
        std::cout << "[GameManager] Scene -> " << (int)targetFaction
                  << " | cache: " << allMaps.size() << "\n";
    }

    void GameManager::run() {
        running = true;
        sf::Clock clock;
        while (window.isOpen() && running) {
            float dt = clock.restart().asSeconds();
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
            if (const auto* r = event->getIf<sf::Event::Resized>()) {
                sf::Vector2f s = { (float)r->size.x, (float)r->size.y };
                window.setView(sf::View(sf::FloatRect({ 0.f, 0.f }, s)));
            }

            window.setView(sceneView);

            bool uiConsumed = (m_hud && m_hud->isMouseOverUI());

            if (const auto* key = event->getIf<sf::Event::KeyPressed>()) {
                if (key->scancode == sf::Keyboard::Scancode::Escape) running = false;

                if (key->scancode == sf::Keyboard::Scancode::F3) {
                    for (auto& [id, map] : allMaps)
                        debugSystem.toggle(map->getGameObjects());
                }
            }

            if (!uiConsumed && currentScene)
                currentScene->handleEvent(window, *event);
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

    GameManager::~GameManager() {}

}
