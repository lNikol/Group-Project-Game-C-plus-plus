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
        m_selector.setOutlineThickness(2.f);

        // Setup Range Indicator
        m_rangeIndicator.setRadius(1.f);
        m_rangeIndicator.setScale({ 1.f, 0.5f });
        m_rangeIndicator.setFillColor(sf::Color(255, 0, 0, 40));
        m_rangeIndicator.setOutlineColor(sf::Color::Red);
        m_rangeIndicator.setOutlineThickness(1.f);
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
                        floorData.push_back(roadLeft);
                    }
                    else if (x == 7) {
                        // Center of the road
                        floorData.push_back(roadMiddle);
                    }
                    else if (x == 8) {
                        // Right edge of the road
                        floorData.push_back(roadRight);
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

    void BattleManager::initTestLevel() {
        // Define the End Turn Logic
        // We capture 'this' so the command can call our private method
        auto endTurnAction = std::make_shared<EndTurnCommand>([this]() {
            this->endTurn();
            });
        auto moveAction = std::make_shared<MoveCommand>(*this);
        // 1. Setup Player
        if (m_playerUnit) {
            m_playerUnit->setLogicalPosition(100.f, 100.f);
            m_playerUnit->setInitiative(20); // Player acts first
            m_playerUnit->setSprite(*m_charTexture, sf::IntRect({ 0, 0 }, { 32, 32 }));
            // Give Fireball (Slot 0)
            if (m_playerUnit->getHotbarAbility(0) == nullptr) {
                auto fireball = std::make_shared<CombatAbility>("Fireball", 96, *this, m_playerUnit.get());
                fireball->setStats(40.f, 250.f, 10.f, 1);
                m_playerUnit->setHotbarAbility(0, fireball);
            }

            // Give End Turn (Slot 17 - Last Slot)
            m_playerUnit->setHotbarAbility(17, endTurnAction);
            m_playerUnit->setHotbarAbility(9, moveAction);
            m_map->addObject(m_playerUnit);
        }

        // 2. Setup Enemy (skeleton)
        Vitals skeletonStats = { 50, 50, 0, 0, 20, 20 };
        auto skeleton = std::make_shared<Unit>("skeleton", Team::Enemy, skeletonStats);
        skeleton->setLogicalPosition(300.f, 200.f);
        skeleton->setInitiative(10); // Acts second
        skeleton->setSprite(*m_monsterTexture, sf::IntRect({ 128, 0 }, { 32, 32 }));
        // Give skeleton a simple attack
        auto slash = std::make_shared<CombatAbility>("Slash", 1, *this, skeleton.get());
        slash->setStats(10.f, 30.f, 0.f, 0);
        skeleton->setHotbarAbility(0, slash);

        // Give skeleton End Turn
        skeleton->setHotbarAbility(17, endTurnAction);
        skeleton->setHotbarAbility(9, moveAction);
        m_map->addObject(skeleton);

        // 3. Props
        auto wall = std::make_shared<Prop>(true, true, "Stone Pillar");
        wall->setLogicalPosition(150.f, 150.f);
        wall->setColliderSize(40.f, 40.f);
        wall->setSprite(*m_propTexture, sf::IntRect({ 32, 32 }, { 32, 32 }));
        m_map->addObject(wall);

        // 4. HUD Init
        if (m_hud) {
            m_hud->onResize({Window::WIDTH, Window::HEIGHT});
        }
        m_map->onResize({ Window::WIDTH, Window::HEIGHT });
        // 5. Start the Battle Loop
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
        auto clickedObj = m_map->getHitObject(logicalPos);
        auto clickedUnit = std::dynamic_pointer_cast<Unit>(clickedObj);

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
        // --- SELECTION MODE ---
        if (clickedUnit) {
            selectUnit(clickedUnit);
        }
        else {
            // Clicked ground -> Deselect
            m_selectedUnit = nullptr;
            m_state = GameState::Idle;
            // TODO: Move command logic here later
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

        // 2. State Management: Busy -> UnitSelected
        if (m_state == GameState::Busy) {
            // Check if the unit we are controlling has finished walking
            if (m_selectedUnit && !m_selectedUnit->isMoving()) {
                // Movement complete, unlock input
                m_state = GameState::UnitSelected;
            }
        }
    }

    void BattleManager::render(sf::RenderWindow& window) {
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
        // 2. Draw Objects (Sorted by Depth)
        // ==============================
        auto objects = m_map->getAllObjects();

        // Sort: Objects "higher" on screen (lower Y) or "behind" drawn first.
        // In Iso, depth = x + y.
        std::sort(objects.begin(), objects.end(), [](const auto& a, const auto& b) {
            return a->getRenderDepth() < b->getRenderDepth();
            });

        for (const auto& obj : objects) {
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
            // Draw Selector Ring UNDER the unit if selected
            // (Actually, to draw under, we should draw before sprite, but for rings it's okay)
            if (auto u = std::dynamic_pointer_cast<Unit>(obj)) {
                if (u == m_selectedUnit) {
                    // Move selector to unit's visual feet
                    m_selector.setPosition(obj->getRenderPosition());
                    window.draw(m_selector);
                }
            }
        }

        // ==============================
        // 3. Draw Range Indicator
        // ==============================
        if (m_state == GameState::TargetingMode && m_selectedUnit && m_pendingAbility) {
            float r = m_pendingAbility->getRange();

            // Adjust radius for visual scale if needed. 
            // Since Iso::worldToScreen squashes Y by 0.5, we scale the circle shape:
            m_rangeIndicator.setRadius(r);
            m_rangeIndicator.setScale({ 1.f, 0.5f });
            m_rangeIndicator.setOrigin({ r, r }); // Center it

            m_rangeIndicator.setPosition(m_selectedUnit->getRenderPosition());
            window.draw(m_rangeIndicator);
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
            float maxDist = m_selectedUnit->getVitals().stamina / STAMINA_COST_PER_UNIT;
            m_rangeIndicator.setRadius(maxDist);
            m_rangeIndicator.setScale({ 1.f, 0.5f });
            m_rangeIndicator.setOrigin({ maxDist, maxDist });
            m_rangeIndicator.setPosition(m_selectedUnit->getRenderPosition());
            m_rangeIndicator.setFillColor(sf::Color::Transparent);
            m_rangeIndicator.setOutlineColor(sf::Color(50, 255, 255, 100)); // Cyan
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
            window.setView(window.getDefaultView());

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

        // 5. Draw HUD
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
}