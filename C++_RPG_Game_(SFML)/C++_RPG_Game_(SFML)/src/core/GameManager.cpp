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
        player = std::make_shared<Player>(lastWorldPosition);

        // Shared player initialization
        Vitals startStats = { 100.f, 100.f, 50.f, 50.f, 100.f, 100.f }; // HP, MP, Stamina
        playerUnit = std::make_shared<Unit>("Hero", Team::Player, startStats);
        // --- 1. MAIN WORLD ---
        auto mainMap = std::make_shared<WorldMap>(GameConfig::WORLD_WIDTH, GameConfig::WORLD_HEIGHT);
        mainMap->generateObstacles(0.07f, player->getPosition(), 3);
        mainMap->placeStructureAtTile(12, 12, "house");


        Animation npcAnimation = Animation::builder()
            .frameCount(8)
            .frameGap(16)
            .frameStartPos({ 16, 16 })
            .spritesheet(AssetManager::getInstance().getSpritesheet("npc"))
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

        // Add a Leader to MainWorld that sends you to WhiteOrder
        addNpc(0, "White Order Envoy", true, FactionID::WhiteOrder, sf::Vector2f{ 520.f, 520.f }, mainMap);
        addNpc(0, "White Orange Order Envoy", false, FactionID::WhiteOrder, sf::Vector2f{ 300.0f, 420.0f }, mainMap);
        addNpc(0, "hite Orange BattleField Envoy", true, FactionID::BattleScene, sf::Vector2f{ 320.0f, 200.0f }, mainMap);

        allMaps[FactionID::MainWorld] = mainMap;

        // --- 2. WHITE ORDER BASE (Small, organized) ---
        auto whiteMap = std::make_shared<WorldMap>(25, 25);
        whiteMap->generateObstacles(0.02f, sf::Vector2f(400, 520), 2);
        //whiteMap->placeStructure(12, 12, StructureType::FactionBase); // Large base in the center
        //whiteMap->placeStructure(10, 10, StructureType::Camp);        // Guard camp nearby

        // NPC to go back to MainWorld
        addNpc(0, "White Order Gatekeeper", true, FactionID::MainWorld, sf::Vector2f{ 560.0f, 560.0f }, whiteMap);
        
        allMaps[FactionID::WhiteOrder] = whiteMap;

        // --- 3. DARK ORDER BASE (Cramped, dangerous) ---
        auto darkMap = std::make_shared<WorldMap>(15, 15);
        // Dark Order has many walls/rocks
        darkMap->generateObstacles(0.15f, sf::Vector2f(400, 480), 2); 

        //darkMap->placeStructure(5, 5, StructureType::Wall);
        //darkMap->placeStructure(5, 6, StructureType::Wall);
        //darkMap->placeStructure(15, 15, StructureType::FactionBase);

        allMaps[FactionID::DarkOrder] = darkMap;

        // --- 4. NEUTRAL ORDER (Wide, empty) ---
        auto neutralMap = std::make_shared<WorldMap>(40, 40);
        neutralMap->generateObstacles(0.25f, sf::Vector2f(440, 620), 4);
        //neutralMap->placeStructure(5, 5, StructureType::Camp);
        //neutralMap->placeStructure(35, 35, StructureType::Camp);

        // Add many trees for "Nature" feel if your generator allows it, 
        // or manually place a few:
        //neutralMap->placeStructure(20, 20, StructureType::Tree);
        //neutralMap->placeStructure(21, 20, StructureType::Tree);

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
            currentScene = std::make_unique<FactionScene>(*this, selectedMap, player);
            break;

        case FactionID::DarkOrder:
            player->setPosition(sf::Vector2f(400.0f, 480.0f));
            currentScene = std::make_unique<FactionScene>(*this, selectedMap, player);
            break;

        case FactionID::NeutralOrder:
            player->setPosition(sf::Vector2f(440.0f, 620.0f));
            currentScene = std::make_unique<FactionScene>(*this, selectedMap, player);
            break;

        case FactionID::BattleScene:
            currentScene = std::make_unique<BattleScene>(*this, window, playerUnit, "assets/jsons/encounters/enc_01_tutorial.json");
            break;

        default:
        case FactionID::MainWorld:
            player->setPosition(sf::Vector2f(lastWorldPosition));
            currentScene = std::make_unique<WorldScene>(*this, selectedMap, player);
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
        
        if (currentScene) currentScene->draw(window);

        //if (m_hud) {
        //    window.draw(*m_hud);
        //}

        window.display();
    }
}