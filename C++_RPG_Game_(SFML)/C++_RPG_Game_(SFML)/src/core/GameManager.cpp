#include "GameManager.h"
#include "scenes/WorldScene.h"
#include "scenes/FactionScene.h"
#include "scenes/BattleScene.h"
 #include "combat/AbilityFactory.h"
 #include "combat/UnitFactory.h"
 #include "combat/PropFactory.h"
#include "combat/PartyData.h"
#include "worldmap/MapLoader.h"
#include "core/QuestManager.h"
#include "core/DialogManager.h"
#include "core/EventBus.h"
#include "game_objects/components/components.h"

namespace RPG {

    GameManager::GameManager()
        : window(sf::VideoMode::getDesktopMode(), Window::TITLE, sf::State::Fullscreen),
          running(false)
    {
        lastWorldPosition = sf::Vector2f(-1.0f, -1.0f);
        activeFaction     = FactionID::MainWorld;
        window.setFramerateLimit(Window::BASE_FPS);

        AssetManager::getInstance().init();
        AssetManager::getInstance().addSpritesheet("player", GameConfig::ANIMATIONS_PATH + "player.png");
        AssetManager::getInstance().addSpritesheet("npc",    GameConfig::ANIMATIONS_PATH + "npc.png");
        AssetManager::getInstance().addSpritesheet("indicator", GameConfig::ANIMATIONS_PATH + "indicator.png");
        AssetManager::getInstance().addTexture("grass",      GameConfig::TEXTURES_PATH   + "grass.png");

        RPG::EventBus::getInstance().subscribe(EventType::QuestCompleted, [this](const RPG::GameEvent& e) {
            const auto& questEvent = static_cast<const QuestCompletedEvent&>(e);
            std::cout << "[GameManager] Received QuestCompletedEvent for: " << questEvent.questId << "\n";
            std::cout << "              Rewards -> XP: " << questEvent.xpReward << ", Gold: " << questEvent.goldReward << "\n";
            
            PartyData::getInstance().gold += questEvent.goldReward;
            std::cout << "[GameManager] Added " << questEvent.goldReward << " gold to Party Inventory. Total: " 
                      << PartyData::getInstance().gold << "\n";
                      
            if (m_systemHud) {
                m_systemHud->notifyQuestUpdated();
            }
        });

        RPG::EventBus::getInstance().subscribe(EventType::QuestStarted, [this](const RPG::GameEvent& e) {
            if (m_systemHud) m_systemHud->notifyQuestUpdated();
        });

        RPG::EventBus::getInstance().subscribe(EventType::QuestReadyToTurnIn, [this](const RPG::GameEvent& e) {
            if (m_systemHud) m_systemHud->notifyQuestUpdated();
        });

        initGameData();

        AssetManager::getInstance().addSpritesheet("AbilityIcons",  GameConfig::TEXTURES_PATH + "placeholders/IconSet.png");
        AssetManager::getInstance().addTexture("ui_window",         GameConfig::TEXTURES_PATH + "placeholders/UI_Window.png");
        AssetManager::getInstance().addTexture("ui_slot",           GameConfig::TEXTURES_PATH + "placeholders/ui_slot.png");
        AssetManager::getInstance().addTexture("close_button",      GameConfig::TEXTURES_PATH + "placeholders/close_button.png");
        AssetManager::getInstance().addFont("PixelFont",            GameConfig::ASSETS_PATH   + "fonts/m5x7.ttf");
        AssetManager::getInstance().addSpritesheet("BattleChars",   GameConfig::TEXTURES_PATH + "placeholders/characters1.png");
        AssetManager::getInstance().addSpritesheet("BattleEnemies", GameConfig::TEXTURES_PATH + "placeholders/Monster1.png");
        AssetManager::getInstance().addSpritesheet("BattleProps",   GameConfig::TEXTURES_PATH + "placeholders/!Other1.png");
        AssetManager::getInstance().addSpritesheet("BattleTiles",   GameConfig::TEXTURES_PATH + "placeholders/Outside_A2.png");
        AssetManager::getInstance().addSpritesheet("BattleBG",      GameConfig::TEXTURES_PATH + "placeholders/Mountains3.png");

        m_globalFont = AssetManager::getInstance().getFont("PixelFont");
        m_iconSet    = AssetManager::getInstance().getSpritesheet("AbilityIcons");

        // Initialize System HUD
        if (m_iconSet && m_globalFont) {
            m_systemHud = std::make_unique<SystemHUD>(*m_iconSet, *m_globalFont);
            m_systemHud->onResize(window.getSize());
        }


        executeSceneChange(FactionID::MainWorld); 
        m_transitionState = TransitionState::FadingIn;
        m_transitionAlpha = 255.f;
    }

    void GameManager::initGameData() {
        if (!AbilityFactory::getInstance().loadFromJSON("assets/jsons/abilities.json")) {
            std::cerr << "[GameManager] CRITICAL: Failed to load abilities.json\n";
        }
        if (!UnitFactory::getInstance().loadFromJSON("assets/jsons/units.json")) {
            std::cerr << "[GameManager] CRITICAL: Failed to load units.json\n";
        }
        if (!PropFactory::getInstance().loadFromJSON("assets/jsons/props.json")) {
            std::cerr << "[GameManager] CRITICAL: Failed to load props.json\n";
        }
        if (!QuestManager::getInstance().loadFromJSON("assets/jsons/quests.json")) {
            std::cerr << "[GameManager] Warning: Failed to load quests.json\n";
        }
        if (!DialogManager::getInstance().loadFromJSON("assets/jsons/dialogs.json")) {
            std::cerr << "[GameManager] Warning: Failed to load dialogs.json\n";
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

        // --- TEST NPC WITH QUEST ---
        auto spawnPos = mainMap->getSpawnPoint();
        spawnPos.x += 64.f; // Place it a bit to the right of the player
        auto testNpc = Factory::createNpc(AssetManager::getInstance(), spawnPos, FactionID::NeutralOrder);
        if (auto* npcComp = testNpc->getComponent<NpcComponent>()) {
            npcComp->setFactionLeader(false);
        }
        testNpc->addComponent<DialogComponent>(1);
        mainMap->addGameObject(std::move(testNpc));

        // --- TEST LIGHT COMMANDER NPC ---
        auto commanderPos = spawnPos;
        commanderPos.x += 64.f; // Place it a bit further to the right
        auto commanderNpc = Factory::createNpc(AssetManager::getInstance(), commanderPos, FactionID::WhiteOrder);
        if (auto* npcComp = commanderNpc->getComponent<NpcComponent>()) {
            npcComp->setFactionLeader(true);
        }
        commanderNpc->addComponent<DialogComponent>(2);
        mainMap->addGameObject(std::move(commanderNpc));
        // ---------------------------

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
        if (m_transitionState == TransitionState::None) {
            m_pendingFaction = targetFaction;
            m_transitionState = TransitionState::FadingOut;
        }
    }

    void GameManager::executeSceneChange(FactionID targetFaction) {
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

            if (event->is<sf::Event::Closed>()) window.close();

            // Block input events during transition
            bool isInputBlocked = (m_transitionState != TransitionState::None);
            if (isInputBlocked) {
                if (event->is<sf::Event::KeyPressed>() || event->is<sf::Event::KeyReleased>() ||
                    event->is<sf::Event::MouseMoved>() || event->is<sf::Event::MouseButtonPressed>() ||
                    event->is<sf::Event::MouseButtonReleased>() || event->is<sf::Event::MouseWheelScrolled>()) 
                {
                    continue; // Drop the event entirely
                }
            }
            if (const auto* r = event->getIf<sf::Event::Resized>()) {
                if (m_systemHud) m_systemHud->onResize(r->size);
            }

            window.setView(sceneView);

            bool uiConsumed = false;
            if (m_systemHud) {
                m_systemHud->handleEvent(window, *event);
                
                // Only block mouse events from reaching the scene if hovering over UI
                if (event->is<sf::Event::MouseMoved>() || 
                    event->is<sf::Event::MouseButtonPressed>() || 
                    event->is<sf::Event::MouseButtonReleased>() ||
                    event->is<sf::Event::MouseWheelScrolled>()) 
                {
                    uiConsumed = m_systemHud->isMouseOverUI();
                }
            }

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
        if (m_systemHud) m_systemHud->update(dt);

        const float TRANSITION_SPEED = 255.f / 0.5f; // 0.5 seconds fade

        if (m_transitionState == TransitionState::FadingOut) {
            m_transitionAlpha += TRANSITION_SPEED * dt;
            if (m_transitionAlpha >= 255.f) {
                m_transitionAlpha = 255.f;
                executeSceneChange(m_pendingFaction);
                m_transitionState = TransitionState::FadingIn;
            }
        }
        else if (m_transitionState == TransitionState::FadingIn) {
            m_transitionAlpha -= TRANSITION_SPEED * dt;
            if (m_transitionAlpha <= 0.f) {
                m_transitionAlpha = 0.f;
                m_transitionState = TransitionState::None;
            }
        }
    }

    void GameManager::draw() {
        window.clear(sf::Color(0x606030FF));
        if (currentScene) currentScene->draw(window);
        if (m_systemHud) window.draw(*m_systemHud);

        // Draw transition overlay
        if (m_transitionAlpha > 0.f) {
            sf::View oldView = window.getView();
            window.setView(window.getDefaultView()); // Draw over entire physical window
            
            sf::RectangleShape overlay((sf::Vector2f)window.getSize());
            overlay.setFillColor(sf::Color(0, 0, 0, static_cast<std::uint8_t>(m_transitionAlpha)));
            window.draw(overlay);
            
            window.setView(oldView);
        }

        window.display();
    }

    bool GameManager::hasWindowFocus() const {
        return window.hasFocus();
    }

    GameManager::~GameManager() {}

}
