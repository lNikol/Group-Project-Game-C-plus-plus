#include "BattleManager.h"
#include "core/IsoHelpers.h"
#include "Prop.h"
#include <iostream>
#include <algorithm> // for std::sort
#include "EndTurnCommand.h"
#include "core/Constants.h"
#include "MoveCommand.h"
#include <iomanip> // for std::setprecision
#include <sstream>
namespace RPG {

    BattleManager::BattleManager(const AssetManager& assetManager, std::shared_ptr<Unit> player)
        : m_assetManager(assetManager), // Store the manager
        m_playerUnit(player),
        m_movementTooltip(*assetManager.getFont("PixelFont")) // Initialize Tooltip directly
    {
        // 1. Fetch Assets "In Place"
        m_iconSet = m_assetManager.getSpritesheet("AbilityIcons");
        m_charTexture = m_assetManager.getSpritesheet("BattleChars");
        m_monsterTexture = m_assetManager.getSpritesheet("BattleEnemies");
        m_propTexture = m_assetManager.getSpritesheet("BattleProps");
        m_tileset  = m_assetManager.getSpritesheet("BattleTiles");
        m_bgTexture = m_assetManager.getSpritesheet("BattleBG");
        m_font = m_assetManager.getFont("PixelFont");

        // Safety Check
        if (!m_iconSet || !m_charTexture || !m_font) {
            std::cerr << "CRITICAL: Missing Battle Assets!" << std::endl;
        }

        m_map = std::make_unique<CombatMap>();

        // 2. Initialize HUD
        // Note: BattleHUD takes references, so we dereference (*) the pointers.
        if (m_iconSet && m_font) {
            m_hud = std::make_unique<BattleHUD>(*m_iconSet, *m_font);
        }
        // Setup Selection Visuals
        m_selector.setRadius(20.f);
        m_selector.setScale({ 1.f, 0.5f }); // Isometric squash
        m_selector.setOrigin({ 20.f, 20.f });
        m_selector.setFillColor(sf::Color::Transparent);
        m_selector.setOutlineColor(sf::Color::Green);
        m_selector.setOutlineThickness(4.f);

        // Setup Range Indicator
        m_rangeIndicator.setRadius(1.f);
        m_rangeIndicator.setScale({ 1.f, 0.5f });
        m_rangeIndicator.setFillColor(sf::Color(255, 0, 0, 40));
        m_rangeIndicator.setOutlineColor(sf::Color::Red);
        m_rangeIndicator.setOutlineThickness(3.f);
        if (m_bgTexture) {
            m_map->setBackground(*m_bgTexture);
        }
        //map
        if (m_tileset) {
            int w = 15;
            int h = 15;
            std::vector<int> floorData;
            floorData.reserve(w * h);

            // Tile Indices
            int grassIndex = 0;
            int roadMiddle = 5;
            int roadLeft = 36;
            int roadRight = 37;

            for (int y = 0; y < h; y++) {
                for (int x = 0; x < w; x++) {

                    if (x == 6) {
                        // Left edge of the road
                        floorData.push_back(roadMiddle);
                    }
                    else if (x == 7) {
                        // Center of the road
                        floorData.push_back(roadMiddle);
                    }
                    else if (x == 8) {
                        // Right edge of the road
                        floorData.push_back(roadMiddle);
                    }
                    else {
                        // Everywhere else is grass
                        floorData.push_back(grassIndex);
                    }
                }
            }

            m_map->loadFromTiles(*m_tileset, { 32, 32 }, w, h, floorData);
        }
    }

    void BattleManager::initTestLevel(const sf::RenderWindow& window) {
        // Define common actions (Move/End Turn)
        auto endTurnAction = std::make_shared<EndTurnCommand>([this]() { this->endTurn(); });
        auto moveAction = std::make_shared<MoveCommand>(*this);

        // ==========================================
        // 1. THE HERO PARTY (Holy Trinity)
        // ==========================================

        // --- A. THE KNIGHT (Tank) ---
        // High HP, High Stamina, Low Initiative
        Vitals vKnight = { 120.f, 120.f, 40.f, 40.f, 50.f, 50.f };
        auto knight = std::make_shared<Unit>("Sir Tankalot", Team::Player, vKnight);

        knight->setLogicalPosition(150.f, 150.f); // Frontline
        knight->setInitiative(10); // Slow
        knight->setCritChance(0.10f);
        knight->setDoubleTurnChance(0.10f);
        knight->setSprite(*m_charTexture, sf::IntRect({ 0, 0 }, { 32, 32 }));

        // Ability 1: Sword Slash
        auto sword = std::make_shared<CombatAbility>("Slash", 115, *this, knight.get());
        sword->setStats(25.f, 40.f, 0.f, 1);
        knight->setHotbarAbility(0, sword);

        // Ability 2: Shield Bash (High Dmg, Cooldown)
        auto bash = std::make_shared<CombatAbility>("Shield Bash", 123, *this, knight.get());
        bash->setStats(45.f, 40.f, 10.f, 3);
        knight->setHotbarAbility(1, bash);

        // --- B. THE ROGUE (DPS) ---
        // Fast, High Crit, Fragile
        Vitals vRogue = { 70.f, 70.f, 30.f, 30.f, 40.f, 40.f };
        auto rogue = std::make_shared<Unit>("Dagger Dan", Team::Player, vRogue);

        rogue->setLogicalPosition(100.f, 180.f); // Flank
        rogue->setInitiative(30); // Acts First
        rogue->setCritChance(0.50f);       // 50% Crit!
        rogue->setDoubleTurnChance(0.30f); // 30% Double Turn!
        rogue->setSprite(*m_charTexture, sf::IntRect({ 96, 0 }, { 32, 32 })); 

        // Ability 1: Quick Shank (Low cost, fish for crits)
        auto shank = std::make_shared<CombatAbility>("Shank", 385, *this, rogue.get());
        shank->setStats(15.f, 40.f, 0.f, 1);
        rogue->setHotbarAbility(0, shank);

        // Ability 2: Throwing Knife (Range)
        auto throwKnife = std::make_shared<CombatAbility>("Throw Knife", 115, *this, rogue.get());
        throwKnife->setStats(20.f, 180.f, 5.f, 1);
        rogue->setHotbarAbility(1, throwKnife);

        // --- C. THE MAGE (Healer/Nuker) ---
        // Squishy, High Mana, Ranged
        Vitals vMage = { 50.f, 50.f, 100.f, 100.f, 30.f, 30.f };
        auto mage = std::make_shared<Unit>("Merlin", Team::Player, vMage);

        mage->setLogicalPosition(80.f, 80.f); // Backline
        mage->setInitiative(15);
        mage->setCritChance(0.10f);
        mage->setDoubleTurnChance(0.10f);
        mage->setSprite(*m_charTexture, sf::IntRect({ 96, 224 }, { 32, 32 }));

        // Ability 1: Fireball
        auto fireball = std::make_shared<CombatAbility>("Fireball", 96, *this, mage.get());
        fireball->setStats(35.f, 250.f, 15.f, 1);
        mage->setHotbarAbility(0, fireball);

        // Ability 2: Greater Heal (Negative Damage)
        auto heal = std::make_shared<CombatAbility>("Heal", 122, *this, mage.get());
        heal->setStats(-40.f, 200.f, 20.f, 2); // Negative = Heal
        mage->setHotbarAbility(1, heal);

        // Add standard moves
        for (auto u : { knight, rogue, mage }) {
            u->setHotbarAbility(9, moveAction);
            u->setHotbarAbility(17, endTurnAction);
            m_map->addObject(u);
        }
        m_playerUnit = knight;

        // ==========================================
        // 2. THE ENEMY SQUAD (Dark Mage & Minions)
        // ==========================================

        // --- Minions: Skeletons ---
        Vitals vSkel = { 40.f, 40.f, 0.f, 0.f, 20.f, 20.f };
        auto spawnSkeleton = [&](float x, float y) {
            auto skel = std::make_shared<Unit>("Skeleton", Team::Enemy, vSkel);
            skel->setLogicalPosition(x, y);
            skel->setInitiative(12);
            skel->setCritChance(0.05f);
            skel->setDoubleTurnChance(0.05f);
            skel->setSprite(*m_monsterTexture, sf::IntRect({ 128, 0 }, { 32, 32 }));

            auto bonk = std::make_shared<CombatAbility>("Bonk", 143, *this, skel.get());
            bonk->setStats(8.f, 30.f, 0.f, 1);
            skel->setHotbarAbility(0, bonk);
            skel->setHotbarAbility(17, endTurnAction);
            skel->setHotbarAbility(9, moveAction);
            m_map->addObject(skel);
            };

        spawnSkeleton(300.f, 200.f);
        spawnSkeleton(320.f, 240.f);
        spawnSkeleton(280.f, 300.f);
        spawnSkeleton(350.f, 180.f);

        // --- Boss: Dark Mage ---
        Vitals vBoss = { 150.f, 150.f, 150.f, 150.f, 30.f, 30.f };
        auto darkMage = std::make_shared<Unit>("Dark Mage", Team::Enemy, vBoss);
        darkMage->setLogicalPosition(400.f, 400.f); // Far back
        darkMage->setInitiative(18); // Faster than Knight/Mage, slower than Rogue
        darkMage->setCritChance(0.05f);
        darkMage->setDoubleTurnChance(0.05f);

        darkMage->setSprite(*m_monsterTexture, sf::IntRect({ 0, 0 }, { 32, 32 }));
        // Boss Ability 1: Shadow Bolt
        auto shadowBolt = std::make_shared<CombatAbility>("Shadow Bolt", 8, *this, darkMage.get());
        shadowBolt->setStats(25.f, 300.f, 10.f, 1);
        darkMage->setHotbarAbility(0, shadowBolt);

        // Boss Ability 2: Life Drain (Big damage)
        auto drain = std::make_shared<CombatAbility>("Life Drain", 1, *this, darkMage.get());
        drain->setStats(50.f, 200.f, 30.f, 4);
        darkMage->setHotbarAbility(1, drain);

        darkMage->setHotbarAbility(17, endTurnAction);
        darkMage->setHotbarAbility(9, moveAction);
        m_map->addObject(darkMage);

        // ==========================================
        // 3. PROPS
        // ==========================================
        auto placePillar = [&](float x, float y) {
            auto prop = std::make_shared<Prop>(true, true, "Pillar");
            prop->setLogicalPosition(x, y);
            prop->setColliderSize(30.f, 30.f);
            prop->setSprite(*m_propTexture, sf::IntRect({ 32, 32 }, { 32, 32 }));
            m_map->addObject(prop);
            };

        // Create a choke point in the middle
        placePillar(200.f, 250.f);
        placePillar(240.f, 220.f);

        // Setup HUD & Map Scaling
        if (m_hud) m_hud->onResize(window.getSize());
        m_map->onResize(window.getSize());

        startBattle();
    }

    void BattleManager::startBattle() {
        m_turnQueue.clear();

        // 1. Collect all Units from the Map
        auto objects = m_map->getAllObjects();
        for (const auto& obj : objects) {
            if (auto unit = std::dynamic_pointer_cast<Unit>(obj)) {
                m_turnQueue.push_back(unit);
            }
        }

        // 2. Sort by Initiative (Higher = First)
        std::sort(m_turnQueue.begin(), m_turnQueue.end(), [](const auto& a, const auto& b) {
            return a->getInitiative() > b->getInitiative();
            });

        if (m_turnQueue.empty()) return;

        std::cout << "Battle Started! Turn Order: ";
        for (const auto& u : m_turnQueue) std::cout << u->getName() << " -> ";
        std::cout << std::endl;

        // 3. Start First Turn
        // We don't call nextTurn() directly because nextTurn rotates. 
        // We just set the front as active.
        m_activeUnit = m_turnQueue.front();
        m_activeUnit->onTurnStart();

        // Sync HUD to the first unit
        if (m_hud) m_hud->setCombatActor(m_activeUnit.get());
        selectUnit(m_activeUnit);
    }

    void BattleManager::nextTurn() {
        if (m_turnQueue.empty()) return;

        // 1. Rotate Queue: Front goes to Back
        auto finishedUnit = m_turnQueue.front();
        m_turnQueue.pop_front();
        m_turnQueue.push_back(finishedUnit);

        // 2. New Active Unit
        m_activeUnit = m_turnQueue.front();

        // 3. Trigger Start Logic (Cooldowns etc)
        m_activeUnit->onTurnStart();

        std::cout << ">>> NEW TURN: " << m_activeUnit->getName() << " <<<" << std::endl;

        // 4. SYNC HUD
        if (m_hud) {
            m_hud->setCombatActor(m_activeUnit.get());
        }

        // 5. Select the unit visually so the camera/player knows who it is
        selectUnit(m_activeUnit);

        // 6. If it is an ENEMY turn, the AI should trigger here.
        if (m_activeUnit->getTeam() == Team::Enemy) {
            // startAI(); // TODO: Add this later.
        }
    }

    void BattleManager::endTurn() {
        // Only allow ending turn if we aren't mid-animation
        if (m_state == GameState::Busy) return;

        // Cancel any pending targeting
        cancelTargeting();
        // --- Double Turn Check ---
        if (m_activeUnit) {
            // Generate random float 0.0 to 1.0
            float roll = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);

            if (roll < m_activeUnit->getDoubleTurnChance()) {
                std::cout << ">>> DOUBLE TURN! " << m_activeUnit->getName() << " acts again! <<<" << std::endl;

                // Refill resources and reduce cooldowns (as if a new turn started)
                m_activeUnit->onTurnStart();

                // Visually re-select the unit to update UI
                selectUnit(m_activeUnit);

                // RETURN EARLY: Do not rotate the queue (nextTurn)
                return;
            }
        }
        nextTurn();
    }

    bool BattleManager::isPlayerTurn() const {
        return m_activeUnit && m_activeUnit->getTeam() == Team::Player;
    }

    void BattleManager::handleEvent(sf::RenderWindow& window, const sf::Event& event) {

        // 1. Handle Window Resizing
        if (const auto* resized = event.getIf<sf::Event::Resized>()) {
            if (m_hud) m_hud->onResize(resized->size);
            if (m_map) m_map->onResize(resized->size);
            return;
        }

        // ==============================
        // PHASE 1: UI HANDLING (Screen Space)
        // ==============================

        // Save the current World View so we can restore it later
        sf::View worldView = window.getView();

        // Switch to UI View (Default 1:1 pixel mapping)
        window.setView(window.getDefaultView());

        bool uiCaptured = false;
        if (m_hud) {
            m_hud->handleEvent(window, event); // Now mapPixelToCoords uses UI space!
            if (m_hud->isMouseOverUI()) {
                uiCaptured = true;
            }
        }

        // ==============================
        // PHASE 2: MAP HANDLING (World Space)
        // ==============================

        // Restore the World View for map clicks
        window.setView(worldView);

        // Only process map if UI didn't catch the click
        if (!uiCaptured) {
            if (const auto* mouseBtn = event.getIf<sf::Event::MouseButtonPressed>()) {
                if (mouseBtn->button == sf::Mouse::Button::Left) {
                    onLeftClick(window, mouseBtn->position);
                }
                else if (mouseBtn->button == sf::Mouse::Button::Right) {
                    onRightClick();
                }
            }

            if (const auto* key = event.getIf<sf::Event::KeyPressed>()) {
                // SPACE -> End turn
                if (key->scancode == sf::Keyboard::Scancode::Space) endTurn();
                // F3 -> debug
                if (const auto* key = event.getIf<sf::Event::KeyPressed>()) {
                    if (key->scancode == sf::Keyboard::Scancode::F3) {
                        m_showDebug = !m_showDebug;
                        std::cout << "Debug Mode: " << (m_showDebug ? "ON" : "OFF") << std::endl;
                    }
                }
            }
        }
    }

    void BattleManager::onLeftClick(const sf::RenderWindow& window, const sf::Vector2i& mousePixel) {
        // 1. Convert Screen Pixels -> World Coordinates using the provided Window
        sf::Vector2f mouseWorld = window.mapPixelToCoords(mousePixel);
        sf::Vector2f logicalPos = Iso::screenToWorld(mouseWorld);

        // 2. Query Map
        std::shared_ptr<Unit> clickedUnit = nullptr;
        // =========================================================
        // SPRITE-BASED SELECTION (Visual)
        // Check if mouse is hovering over the actual sprite image
        // =========================================================
        auto allObjects = m_map->getAllObjects();

        // Sort by Render Depth Descending (Front to Back)
        // We want to check the object "closest" to the camera first.
        std::sort(allObjects.begin(), allObjects.end(), [](const auto& a, const auto& b) {
            return a->getRenderDepth() > b->getRenderDepth();
            });

        for (const auto& obj : allObjects) {
            auto unit = std::dynamic_pointer_cast<Unit>(obj);
            if (!unit) continue;

            if (const auto* sprite = unit->getSprite()) {
                // getGlobalBounds returns the rect in World Coordinates (where the sprite is drawn)
                if (sprite->getGlobalBounds().contains(mouseWorld)) {
                    clickedUnit = unit;
                    break; // Found the top-most unit
                }
            }
        }

        // =========================================================
        // 2. FALLBACK: TILE-BASED SELECTION (Logical)
        // If we didn't click a sprite body, did we click their feet?
        // =========================================================
        if (!clickedUnit) {
            auto clickedObj = m_map->getHitObject(logicalPos);
            clickedUnit = std::dynamic_pointer_cast<Unit>(clickedObj);
        }
        // --- TARGETING MODE ---
        if (m_state == GameState::TargetingMode && m_pendingAbility) {
            if (clickedUnit) {
                float dist = Iso::getDistance(m_selectedUnit->getLogicalPosition(), clickedUnit->getLogicalPosition());

                if (dist <= m_pendingAbility->getRange()) {
                    m_pendingAbility->resolve(clickedUnit);
                    m_pendingAbility = nullptr;
                    m_state = GameState::UnitSelected;
                }
                else {
                    std::cout << "Target out of range!" << std::endl;
                }
            }
            else {
                std::cout << "Invalid Target." << std::endl;
            }
            return;
        }
        // --- MOVEMENT MODE ---
        if (m_state == GameState::Moving && m_selectedUnit) {
            float cost = 0.f;

            if (isValidMove(logicalPos, cost)) {
                // DO THE MOVE
                m_selectedUnit->consumeStamina(cost);
                m_selectedUnit->moveTo(logicalPos);
                m_state = GameState::Busy;
            }
            else {
                std::cout << "Invalid Move!" << std::endl;
            }
            return; // Stop processing click
        }
    }

    void BattleManager::onRightClick() {
        if (m_state == GameState::TargetingMode) {
            cancelTargeting();
        }
        else {
            m_selectedUnit = nullptr;
            m_state = GameState::Idle;
        }
    }

    void BattleManager::selectUnit(std::shared_ptr<Unit> unit) {
        m_selectedUnit = unit;
        m_state = GameState::UnitSelected;

        // Sync HUD
        if (m_hud) {
            m_hud->setCombatActor(unit.get());
        }
    }

    void BattleManager::startTargeting(CombatAbility* ability) {
        if (!ability || !m_selectedUnit) return;
        m_pendingAbility = ability;
        m_state = GameState::TargetingMode;
        std::cout << "Targeting Mode: " << ability->getRange() << " range." << std::endl;
    }

    void BattleManager::cancelTargeting() {
        m_pendingAbility = nullptr;
        if (m_selectedUnit) m_state = GameState::UnitSelected;
        else m_state = GameState::Idle;
    }

    void BattleManager::update(float dt) {
        if (m_hud) m_hud->update(dt);
        // 1. Update all Units (Physics/Animation)
        // We iterate over map objects to find units
        for (const auto& obj : m_map->getAllObjects()) {
            if (auto unit = std::dynamic_pointer_cast<Unit>(obj)) {
                unit->update(dt);
            }
        }
        // 2. Update Floating Texts
        // Iterate backwards to allow safe removal
        for (int i = m_floatingTexts.size() - 1; i >= 0; i--) {
            auto& ft = m_floatingTexts[i];

            // 1. Physics
            ft.position += ft.velocity * dt;
            ft.text.setPosition(ft.position);

            // 2. Age
            ft.lifetime -= dt;

            // 3. Fade Out
            float ratio = ft.lifetime / ft.maxLifetime;
            if (ratio < 0) ratio = 0;

            sf::Color c = ft.text.getFillColor();
            c.a = static_cast<uint8_t>(255 * ratio);
            ft.text.setFillColor(c);

            sf::Color o = ft.text.getOutlineColor();
            o.a = static_cast<uint8_t>(255 * ratio);
            ft.text.setOutlineColor(o);

            // 4. Cleanup
            if (ft.lifetime <= 0.f) {
                m_floatingTexts.erase(m_floatingTexts.begin() + i);
            }
        }
        // 3. State Management: Busy -> UnitSelected
        if (m_state == GameState::Busy) {
            // Check if the unit we are controlling has finished walking
            if (m_selectedUnit && !m_selectedUnit->isMoving()) {
                // Movement complete, unlock input
                m_state = GameState::UnitSelected;
            }
        }
    }

    void BattleManager::render(sf::RenderWindow& window) {
        const float VISUAL_CORRECTION = 1.31f;
        //grid logic
        const int gridSize = 15;
        const float tileSize = 32.f;

        // ==========================================================
        // Center the Camera on the Grid
        // ==========================================================

        // 1. Calculate the center of the map in World Coordinates
        float midWorld = (gridSize * tileSize) / 2.0f;

        // 2. Convert that center point to Screen Coordinates
        sf::Vector2f mapCenterScreen = Iso::worldToScreen({ midWorld, midWorld });

        // 3. Create a view centered on that point
        // Get the current window size to ensure aspect ratio is correct
        sf::Vector2f windowSize = sf::Vector2f(window.getSize());
        sf::View cameraView(mapCenterScreen, windowSize);

        // 4. Apply the view (Zoom out slightly if you want to see more)
        // cameraView.zoom(1.2f); 
        window.setView(cameraView);
        // ==============================
        // 1. Draw Floor & Background
        // ==============================
        window.draw(*m_map);

        if(m_showDebug)
        {
            sf::VertexArray gridLines(sf::PrimitiveType::Lines);
            sf::Color gridColor(255, 255, 255, 80); // Semi-transparent white

            for (int x = 0; x <= gridSize; ++x) {
                sf::Vector2f start = Iso::worldToScreen({ x * tileSize, 0.f });
                sf::Vector2f end = Iso::worldToScreen({ x * tileSize, gridSize * tileSize });
                gridLines.append(sf::Vertex({ start, gridColor }));
                gridLines.append(sf::Vertex({ end, gridColor }));
            }
            for (int y = 0; y <= gridSize; ++y) {
                sf::Vector2f start = Iso::worldToScreen({ 0.f, y * tileSize });
                sf::Vector2f end = Iso::worldToScreen({ gridSize * tileSize, y * tileSize });
                gridLines.append(sf::Vertex({ start, gridColor }));
                gridLines.append(sf::Vertex({ end, gridColor }));
            }
            window.draw(gridLines);
        }


        // ==============================
        // 2. Draw Range Indicator
        // ==============================
        if (m_state == GameState::TargetingMode && m_selectedUnit && m_pendingAbility) {
            float r = m_pendingAbility->getRange()*VISUAL_CORRECTION;

            m_rangeIndicator.setRadius(r);
            m_rangeIndicator.setScale({ 1.f, 0.5f });
            m_rangeIndicator.setOrigin({ r, r }); // Center it

            m_rangeIndicator.setPosition(m_selectedUnit->getRenderPosition());
            window.draw(m_rangeIndicator);
        }

        // ==============================
        // 3. Draw Objects (Sorted by Depth)
        // ==============================
        auto objects = m_map->getAllObjects();

        // Sort: Objects "higher" on screen (lower Y) or "behind" drawn first.
        // In Iso, depth = x + y.
        std::sort(objects.begin(), objects.end(), [](const auto& a, const auto& b) {
            return a->getRenderDepth() < b->getRenderDepth();
            });

        for (const auto& obj : objects) {
            // Draw Health Bar and Selector Ring UNDER the unit if selected
            if (auto u = std::dynamic_pointer_cast<Unit>(obj)) {
                if (u == m_selectedUnit) {
                    // Move selector to unit's visual feet
                    m_selector.setPosition(obj->getRenderPosition());
                    window.draw(m_selector);
                }
                // Draw Health Bar above units
                const auto* sprite = obj->getSprite();

                float hp = u->getVitals().hp;
                float maxHp = u->getVitals().maxHp;
                float ratio = hp / maxHp;

                // Get sprite position
                sf::Vector2f pos = sprite->getPosition();

                float barWidth = 40.f;
                float barHeight = 5.f;
                float offsetY = -70.f;

                // black part
                sf::RectangleShape bg;
                bg.setSize({ barWidth, barHeight });
                bg.setFillColor(sf::Color::Black);
                bg.setOrigin({ barWidth / 2.f, barHeight / 2.f });
                bg.setPosition({ pos.x, pos.y + offsetY });

                // red part
                sf::RectangleShape hpBar;
                hpBar.setSize({ barWidth * ratio, barHeight });
                hpBar.setFillColor(sf::Color::Red);

                hpBar.setOrigin({ 0.f, barHeight / 2.f });
                hpBar.setPosition({pos.x - (barWidth / 2.f),  pos.y + offsetY});

                window.draw(bg);
                window.draw(hpBar);     
            }

            // Recalculate sprite position based on logical position
            obj->updateVisuals();

            if (const auto* sprite = obj->getSprite()) {
                window.draw(*sprite);

            }
            // Fallback
            else {
                sf::CircleShape debug(10.f);
                debug.setPosition(obj->getRenderPosition());
                window.draw(debug);
            }
        }
        // draws turn sequence of units to play in the left corner
        float startX = -580.0f;
        float startY = -150.0f;
        float spacing = 60.0f;
        for (size_t i = 0; i < m_turnQueue.size(); i++)
        {
            auto& unit = m_turnQueue[i];

            const sf::Sprite* sprite = unit->getSprite();
            if (!sprite) continue;

            sf::Sprite icon = *sprite;

            icon.setPosition({ startX + i * spacing, startY });
            icon.setScale({ 1.3f, 1.3f });
            icon.setOrigin({ 0.f, 0.f });

            // Draw red circle for FIRST unit
            if (i == 0)
            {
                sf::CircleShape highlight;
                highlight.setRadius(25.f);
                highlight.setFillColor(sf::Color::Transparent);
                highlight.setOutlineColor(sf::Color::Red);
                highlight.setOutlineThickness(3.f);

                highlight.setOrigin({ 25.f, 25.f });

                // center it on icon
                highlight.setPosition({
                    startX + i * spacing + 21.f,
                    startY + 21.f
                    });

                window.draw(highlight);
            }

            window.draw(icon);
        }

        // ==========================================================
        // Draw Movement Visuals
        // ==========================================================
        if (m_state == GameState::Moving && m_selectedUnit) {
            // --- WORLD SPACE CALCULATIONS ---;
            sf::Vector2i mousePixel = sf::Mouse::getPosition(window);
            sf::Vector2f mouseWorld = window.mapPixelToCoords(mousePixel); // Uses current CameraView
            sf::Vector2f mouseLogic = Iso::screenToWorld(mouseWorld);
            // 1. Draw Max Range Circle (In World Space)
            float maxDist = m_selectedUnit->getVitals().stamina / STAMINA_COST_PER_UNIT*VISUAL_CORRECTION;
            m_rangeIndicator.setRadius(maxDist);
            m_rangeIndicator.setScale({ 1.f, 0.5f });
            m_rangeIndicator.setOrigin({ maxDist, maxDist });
            m_rangeIndicator.setPosition(m_selectedUnit->getRenderPosition());
            m_rangeIndicator.setFillColor(sf::Color::Transparent);
            m_rangeIndicator.setOutlineColor(sf::Color(50, 255, 255, 100)); // Cyan
            m_rangeIndicator.setOutlineThickness(3.f);
            window.draw(m_rangeIndicator);

            // 2. Draw Trajectory Line (In World Space)
            float cost = 0.f;
            bool valid = isValidMove(mouseLogic, cost);
            sf::Color lineColor = valid ? sf::Color::Green : sf::Color::Red;

            sf::VertexArray line(sf::PrimitiveType::Lines, 2);
            line[0].position = m_selectedUnit->getRenderPosition(); // Feet
            line[0].color = lineColor;
            line[1].position = mouseWorld; // Cursor
            line[1].color = lineColor;
            window.draw(line);

            // --- SCREEN SPACE DRAWING ---
            // Switch to Default View so Tooltip text isn't affected by Camera Zoom/Pan
            //window.setView(window.getDefaultView());
            sf::Vector2f currentWindowSize((float)window.getSize().x, (float)window.getSize().y);
            sf::View uiView(sf::FloatRect({ 0.f, 0.f }, currentWindowSize));
            window.setView(uiView);
            std::stringstream ss;
            if (valid) {
                ss << (int)cost << " STAMINA";
            }
            else {
                // If it's invalid because dist is small (clicking self), don't show "Blocked"
                float dist = Iso::getDistance(m_selectedUnit->getLogicalPosition(), mouseLogic);
                if (dist < 10.0f) {
                    ss << ""; // Don't show tooltip on self
                }
                else {
                    ss << "Blocked";
                }
            }

            // Update Tooltip using raw pixel coordinates
            m_movementTooltip.update(ss.str(), sf::Vector2f(mousePixel), windowSize);

            window.draw(m_movementTooltip);

            // Restore Camera View (just in case subsequent code needs it, 
            // though HUD resets it anyway)
            window.setView(cameraView);
        }

        // ==========================================================
        // 4. Debug Overlay
        // ==========================================================
        if (m_showDebug) {
            // Draw collisions for ALL objects
            for (const auto& obj : m_map->getAllObjects()) {

                // 1. Draw Hitbox (Magenta)
                // This shows exactly where the object "exists" on the floor
                drawDebugBox(window, obj->getCollider(), sf::Color::Magenta);

                // 2. Draw Center Point (Yellow Dot)
                // Helps verify logical position vs visual position
                sf::CircleShape centerDot(2.f);
                centerDot.setOrigin({ 1.f, 1.f });
                centerDot.setPosition(obj->getRenderPosition()); // This uses Iso::worldToScreen(position)
                centerDot.setFillColor(sf::Color::Yellow);
                window.draw(centerDot);
            }
        }


        // 5. Floating Text
        for (const auto& ft : m_floatingTexts) {
            window.draw(ft.text);
        }

        // 6. Draw HUD
        if (!m_hasInitializedHUD && m_hud) {
            m_hud->onResize(window.getSize());
            m_hasInitializedHUD = true;
        }
        if (m_hud) {
            window.draw(*m_hud);
        }
    }

    void BattleManager::startMovementMode() {
        if (m_state == GameState::Busy || !m_selectedUnit) return;
        m_state = GameState::Moving;
    }

    // Helper to calculate cost and validity
    bool BattleManager::isValidMove(const sf::Vector2f& target, float& outCost) {
        if (!m_selectedUnit) return false;

        sf::Vector2f start = m_selectedUnit->getLogicalPosition();
        float dist = Iso::getDistance(start, target);

        // Prevent clicking on self (Minimum distance check)
        // If the click is within 10 pixels (approx unit radius), treat as invalid.
        if (dist < 10.0f) {
            outCost = 0.f;
            return false;
        }

        outCost = dist * STAMINA_COST_PER_UNIT;

        // Check Stamina
        if (m_selectedUnit->getVitals().stamina < outCost) return false;

        // Check map
        if (!m_map->canMoveDirectly(start, target, m_selectedUnit.get())) return false;

        return true;
    }

    //Debug helper
    void BattleManager::drawDebugBox(sf::RenderWindow& window, const sf::FloatRect& rect, sf::Color color) {
        // 1. Get the 4 logical corners of the footprint
        sf::Vector2f p1 = { rect.position.x, rect.position.y };
        sf::Vector2f p2 = { rect.position.x + rect.size.x, rect.position.y };
        sf::Vector2f p3 = { rect.position.x + rect.size.x, rect.position.y + rect.size.y };
        sf::Vector2f p4 = { rect.position.x, rect.position.y + rect.size.y };

        // 2. Project them to Isometric Screen Space
        sf::Vector2f s1 = Iso::worldToScreen(p1);
        sf::Vector2f s2 = Iso::worldToScreen(p2);
        sf::Vector2f s3 = Iso::worldToScreen(p3);
        sf::Vector2f s4 = Iso::worldToScreen(p4);

        // 3. Draw lines connecting them
        sf::VertexArray lines(sf::PrimitiveType::LineStrip, 5);
        lines[0] = sf::Vertex(s1, color);
        lines[1] = sf::Vertex(s2, color);
        lines[2] = sf::Vertex(s3, color);
        lines[3] = sf::Vertex(s4, color);
        lines[4] = sf::Vertex(s1, color); // Loop back to start

        window.draw(lines);
    }

    void BattleManager::onUnitDeath(std::shared_ptr<Unit> deadUnit) {
        std::cout << deadUnit->getName() << " has died!" << std::endl;

        // 1. Remove from Map (Visuals & Physics)
        m_map->removeObject(deadUnit);

        // 2. Remove from Turn Queue (Logic)
        // std::remove in a deque is a bit messy, but essential
        auto it = std::remove(m_turnQueue.begin(), m_turnQueue.end(), deadUnit);
        m_turnQueue.erase(it, m_turnQueue.end());

        // 3. Check for Win/Loss
        checkBattleStatus();
    }

    //Victory helper
    void BattleManager::checkBattleStatus() {
        bool playersAlive = false;
        bool enemiesAlive = false;

        // Check who is left in the Turn Queue (or Map)
        for (const auto& unit : m_turnQueue) {
            if (unit->getTeam() == Team::Player) playersAlive = true;
            if (unit->getTeam() == Team::Enemy) enemiesAlive = true;
        }

        if (!enemiesAlive) {
            std::cout << "VICTORY! All enemies defeated." << std::endl;
            // TODO: showVictoryScreen
            m_battleOver = true;
        }
        else if (!playersAlive) {
            std::cout << "DEFEAT! Player has fallen." << std::endl;
            // TODO: showDefeatScreen
            m_battleOver = true;
        }
    }

    void BattleManager::spawnFloatingText(sf::Vector2f location, std::string content, sf::Color color, int fontSize, sf::Vector2f velocity) {
        if (!m_font) return;

        // SFML 3: Font must be passed to constructor
        FloatingText ft(*m_font);

        ft.text.setString(content);
        ft.text.setCharacterSize(fontSize);
        ft.text.setFillColor(color);

        // Outline for readability
        ft.text.setOutlineColor(sf::Color::Black);
        ft.text.setOutlineThickness(1.5f);

        // Center origin
        sf::FloatRect bounds = ft.text.getLocalBounds();
        ft.text.setOrigin({bounds.size.x / 2.f, bounds.size.y / 2.f});

        ft.position = location;
        ft.position.y -= 50.f; // Offset to appear slightly above the anchor point
        ft.velocity = velocity;
        ft.lifetime = 1.2f;
        ft.maxLifetime = 1.2f;

        m_floatingTexts.push_back(ft);
    }
}