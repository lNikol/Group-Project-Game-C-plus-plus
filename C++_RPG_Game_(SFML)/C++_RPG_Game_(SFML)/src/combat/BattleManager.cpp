#include "BattleManager.h"
#include "AbilityFactory.h" // Added for Data-Driven loading
#include "core/IsoHelpers.h"
#include "Prop.h"
#include <iostream>
#include <algorithm> 
#include "EndTurnCommand.h"
#include "core/Constants.h"
#include "MoveCommand.h"
#include <iomanip> 
#include <sstream>

namespace RPG {

    BattleManager::BattleManager(std::shared_ptr<Unit> player)
        : m_playerUnit(player),
        m_movementTooltip(*AssetManager::getInstance().getFont("PixelFont")) // Initialize Tooltip directly
    {
        // 1. Fetch Assets "In Place"
        m_iconSet = AssetManager::getInstance().getSpritesheet("AbilityIcons");
        m_charTexture = AssetManager::getInstance().getSpritesheet("BattleChars");
        m_monsterTexture = AssetManager::getInstance().getSpritesheet("BattleEnemies");
        m_propTexture = AssetManager::getInstance().getSpritesheet("BattleProps");
        m_tileset  = AssetManager::getInstance().getSpritesheet("BattleTiles");
        m_bgTexture = AssetManager::getInstance().getSpritesheet("BattleBG");
        m_font = AssetManager::getInstance().getFont("PixelFont");

        if (!m_iconSet || !m_charTexture || !m_font) {
            std::cerr << "CRITICAL: Missing Battle Assets!" << std::endl;
        }

        m_map = std::make_unique<CombatMap>();

        if (m_iconSet && m_font) {
            m_hud = std::make_unique<BattleHUD>(*m_iconSet, *m_font);
        }

        m_selector.setRadius(20.f);
        m_selector.setScale({ 1.f, 0.5f });
        m_selector.setOrigin({ 20.f, 20.f });
        m_selector.setFillColor(sf::Color::Transparent);
        m_selector.setOutlineColor(sf::Color::Green);
        m_selector.setOutlineThickness(4.f);

        m_rangeIndicator.setRadius(1.f);
        m_rangeIndicator.setScale({ 1.f, 0.5f });
        m_rangeIndicator.setFillColor(sf::Color(255, 0, 0, 40));
        m_rangeIndicator.setOutlineColor(sf::Color::Red);
        m_rangeIndicator.setOutlineThickness(3.f);

        m_aoeIndicator.setFillColor(sf::Color(255, 165, 0, 60)); // Orange
        m_aoeIndicator.setOutlineColor(sf::Color::Yellow);

        if (m_bgTexture) m_map->setBackground(*m_bgTexture);

        if (m_tileset) {
            int w = 15;
            int h = 15;
            std::vector<int> floorData;
            floorData.reserve(w * h);

            int grassIndex = 0;
            int roadMiddle = 5;

            for (int y = 0; y < h; y++) {
                for (int x = 0; x < w; x++) {
                    if (x >= 6 && x <= 8) floorData.push_back(roadMiddle);
                    else floorData.push_back(grassIndex);
                }
            }

            m_map->loadFromTiles(*m_tileset, { 32, 32 }, w, h, floorData);
        }
    }

    void BattleManager::initTestLevel(const sf::RenderWindow& window) {
        // 1. Load Abilities from JSON!
        if (!AbilityFactory::getInstance().loadFromJSON("assets/abilities.json")) {
            std::cerr << "Failed to load combat abilities. Check assets/abilities.json" << std::endl;
        }

        auto endTurnAction = std::make_shared<EndTurnCommand>([this]() { this->endTurn(); });
        auto moveAction = std::make_shared<MoveCommand>(*this);

        Vitals vKnight = { 120.f, 120.f, 40.f, 40.f, 50.f, 50.f };
        auto knight = std::make_shared<Unit>("Sir Tankalot", Team::Player, vKnight);
        knight->setLogicalPosition(150.f, 150.f);
        knight->setInitiative(10);
        knight->setCritChance(0.10f);
        knight->setSprite(*m_charTexture, sf::IntRect({ 0, 0 }, { 32, 32 }));

        // Bind using the Factory
        auto sword = AbilityFactory::getInstance().createAbility("slash", *this, knight.get());
        if (sword) knight->setHotbarAbility(0, sword);

        // (You can bind more Factory abilities for Rogue/Mage here similarly)
        knight->setHotbarAbility(9, moveAction);
        knight->setHotbarAbility(17, endTurnAction);
        m_map->addObject(knight);
        m_playerUnit = knight;

        Vitals vBoss = { 150.f, 150.f, 150.f, 150.f, 30.f, 30.f };
        auto darkMage = std::make_shared<Unit>("Dark Mage", Team::Enemy, vBoss);
        darkMage->setLogicalPosition(400.f, 400.f);
        darkMage->setInitiative(18);
        darkMage->setSprite(*m_monsterTexture, sf::IntRect({ 0, 0 }, { 32, 32 }));

        auto bossSpell = AbilityFactory::getInstance().createAbility("poison_dart", *this, darkMage.get());
        if (bossSpell) darkMage->setHotbarAbility(0, bossSpell);

        darkMage->setHotbarAbility(17, endTurnAction);
        darkMage->setHotbarAbility(9, moveAction);
        m_map->addObject(darkMage);

        auto placePillar = [&](float x, float y) {
            auto prop = std::make_shared<Prop>(true, true, "Pillar");
            prop->setLogicalPosition(x, y);
            prop->setColliderSize(30.f, 30.f);
            prop->setSprite(*m_propTexture, sf::IntRect({ 32, 32 }, { 32, 32 }));
            m_map->addObject(prop);
            };

        placePillar(200.f, 250.f);
        placePillar(240.f, 220.f);

        if (m_hud) m_hud->onResize(window.getSize());
        m_map->onResize(window.getSize());

        startBattle();
    }

    void BattleManager::startBattle() {
        m_turnQueue.clear();
        auto objects = m_map->getAllObjects();
        for (const auto& obj : objects) {
            if (auto unit = std::dynamic_pointer_cast<Unit>(obj)) {
                m_turnQueue.push_back(unit);
            }
        }

        std::sort(m_turnQueue.begin(), m_turnQueue.end(), [](const auto& a, const auto& b) {
            return a->getInitiative() > b->getInitiative();
            });

        if (m_turnQueue.empty()) return;

        m_activeUnit = m_turnQueue.front();
        m_activeUnit->onTurnStart(*this);

        if (m_hud) m_hud->setCombatActor(m_activeUnit.get());
        selectUnit(m_activeUnit);
    }

    void BattleManager::nextTurn() {
        if (m_turnQueue.empty()) return;

        auto finishedUnit = m_turnQueue.front();
        m_turnQueue.pop_front();
        m_turnQueue.push_back(finishedUnit);

        m_activeUnit = m_turnQueue.front();

        // Trigger Start Logic (Cooldowns, DoTs, HoTs)
        m_activeUnit->onTurnStart(*this);

        // Safety Check: Did the unit die from Poison/DoTs at the start of their turn?
        if (m_activeUnit->isDead()) {
            std::cout << m_activeUnit->getName() << " succumbed to their wounds!" << std::endl;
            onUnitDeath(m_activeUnit);
            endTurn();
            return;
        }

        // Stun Check: If stunned, they lose their action phase entirely.
        if (m_activeUnit->isStunned()) {
            std::cout << m_activeUnit->getName() << " is Stunned and loses their turn!" << std::endl;
            spawnFloatingText(m_activeUnit->getRenderPosition(), "Turn Skipped!", sf::Color::White, 20);
            endTurn();
            return;
        }

        if (m_hud) m_hud->setCombatActor(m_activeUnit.get());
        selectUnit(m_activeUnit);
    }

    void BattleManager::endTurn() {
        if (m_state == GameState::Busy) return;
        cancelTargeting();

        if (m_activeUnit) {
            float roll = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
            if (roll < m_activeUnit->getDoubleTurnChance()) {
                m_activeUnit->onTurnStart(*this);
                selectUnit(m_activeUnit);
                return;
            }
        }
        nextTurn();
    }

    bool BattleManager::isPlayerTurn() const {
        return m_activeUnit && m_activeUnit->getTeam() == Team::Player;
    }

    void BattleManager::handleEvent(sf::RenderWindow& window, const sf::Event& event) {
        if (const auto* resized = event.getIf<sf::Event::Resized>()) {
            if (m_hud) m_hud->onResize(resized->size);
            if (m_map) m_map->onResize(resized->size);
            return;
        }

        sf::View worldView = window.getView();
        window.setView(window.getDefaultView());

        bool uiCaptured = false;
        if (m_hud) {
            m_hud->handleEvent(window, event);
            if (m_hud->isMouseOverUI()) uiCaptured = true;
        }

        window.setView(worldView);

        if (!uiCaptured) {
            if (const auto* mouseBtn = event.getIf<sf::Event::MouseButtonPressed>()) {
                if (mouseBtn->button == sf::Mouse::Button::Left) onLeftClick(window, mouseBtn->position);
                else if (mouseBtn->button == sf::Mouse::Button::Right) onRightClick();
            }

            if (const auto* key = event.getIf<sf::Event::KeyPressed>()) {
                if (key->scancode == sf::Keyboard::Scancode::Space) endTurn();
                if (key->scancode == sf::Keyboard::Scancode::F3) m_showDebug = !m_showDebug;
            }
        }
    }

    void BattleManager::onLeftClick(const sf::RenderWindow& window, const sf::Vector2i& mousePixel) {
        sf::Vector2f mouseWorld = window.mapPixelToCoords(mousePixel);
        sf::Vector2f logicalPos = Iso::screenToWorld(mouseWorld);

        std::shared_ptr<Unit> clickedUnit = nullptr;
        auto allObjects = m_map->getAllObjects();

        std::sort(allObjects.begin(), allObjects.end(), [](const auto& a, const auto& b) {
            return a->getRenderDepth() > b->getRenderDepth();
            });

        for (const auto& obj : allObjects) {
            auto unit = std::dynamic_pointer_cast<Unit>(obj);
            if (!unit) continue;
            if (const auto* sprite = unit->getSprite()) {
                if (sprite->getGlobalBounds().contains(mouseWorld)) {
                    clickedUnit = unit;
                    break;
                }
            }
        }

        if (!clickedUnit) {
            auto clickedObj = m_map->getHitObject(logicalPos);
            clickedUnit = std::dynamic_pointer_cast<Unit>(clickedObj);
        }

        // --- TARGETING MODE ---
        if (m_state == GameState::TargetingMode && m_pendingAbility) {
            float castDist = Iso::getDistance(m_selectedUnit->getLogicalPosition(), logicalPos);
            if (castDist > m_pendingAbility->getRange()) {
                std::cout << "Target out of range!" << std::endl;
                return;
            }

            std::vector<std::shared_ptr<Unit>> finalTargets;
            TargetType targetType = m_pendingAbility->getTargetType();

            // PATH A: AOE Targeting
            if (m_pendingAbility->getRadius() > 0.f) {
                auto caughtUnits = m_map->getUnitsInRadius(logicalPos, m_pendingAbility->getRadius());
                for (auto& u : caughtUnits) {
                    bool isAlly = (u->getTeam() == m_selectedUnit->getTeam());
                    if (targetType == TargetType::AreaEnemy && !isAlly) finalTargets.push_back(u);
                    else if (targetType == TargetType::AreaAlly && isAlly) finalTargets.push_back(u);
                }
            }
            // PATH B: Single Targeting
            else {
                if (clickedUnit) {
                    bool isAlly = (clickedUnit->getTeam() == m_selectedUnit->getTeam());
                    if ((targetType == TargetType::SingleEnemy && !isAlly) ||
                        (targetType == TargetType::SingleAlly && isAlly) ||
                        (targetType == TargetType::Self && clickedUnit == m_selectedUnit))
                    {
                        finalTargets.push_back(clickedUnit);
                    }
                    else {
                        std::cout << "Invalid Target Alignment!" << std::endl;
                        return;
                    }
                }
                else {
                    std::cout << "You must click on a valid unit!" << std::endl;
                    return;
                }
            }

            if (!finalTargets.empty()) {
                m_pendingAbility->resolve(finalTargets);
                m_pendingAbility = nullptr;
                m_state = GameState::UnitSelected;
            }
            else {
                std::cout << "No valid targets in that area." << std::endl;
            }
            return;
        }

        // --- MOVEMENT MODE ---
        if (m_state == GameState::Moving && m_selectedUnit) {
            float cost = 0.f;

            if (isValidMove(logicalPos, cost)) {
                m_selectedUnit->consumeStamina(cost);
                std::vector<sf::Vector2f> path = m_map->findPath(m_selectedUnit->getLogicalPosition(), logicalPos, m_selectedUnit.get());
                m_selectedUnit->setPath(path);
                m_state = GameState::Busy;
            }
            else {
                std::cout << "Invalid Move!" << std::endl;
            }
            return;
        }
    }

    void BattleManager::onRightClick() {
        if (m_state == GameState::TargetingMode) cancelTargeting();
        else {
            m_selectedUnit = nullptr;
            m_state = GameState::Idle;
        }
    }

    void BattleManager::selectUnit(std::shared_ptr<Unit> unit) {
        m_selectedUnit = unit;
        m_state = GameState::UnitSelected;
        if (m_hud) m_hud->setCombatActor(unit.get());
    }

    void BattleManager::startTargeting(CombatAbility* ability) {
        if (!ability || !m_selectedUnit) return;
        m_pendingAbility = ability;
        m_state = GameState::TargetingMode;
    }

    void BattleManager::cancelTargeting() {
        m_pendingAbility = nullptr;
        if (m_selectedUnit) m_state = GameState::UnitSelected;
        else m_state = GameState::Idle;
    }

    void BattleManager::update(float dt) {
        if (m_hud) m_hud->update(dt);
        for (const auto& obj : m_map->getAllObjects()) {
            if (auto unit = std::dynamic_pointer_cast<Unit>(obj)) unit->update(dt);
        }

        for (int i = m_floatingTexts.size() - 1; i >= 0; i--) {
            auto& ft = m_floatingTexts[i];
            ft.position += ft.velocity * dt;
            ft.text.setPosition(ft.position);
            ft.lifetime -= dt;

            float ratio = std::max(0.f, ft.lifetime / ft.maxLifetime);
            sf::Color c = ft.text.getFillColor();
            c.a = static_cast<uint8_t>(255 * ratio);
            ft.text.setFillColor(c);

            sf::Color o = ft.text.getOutlineColor();
            o.a = static_cast<uint8_t>(255 * ratio);
            ft.text.setOutlineColor(o);

            if (ft.lifetime <= 0.f) m_floatingTexts.erase(m_floatingTexts.begin() + i);
        }

        if (m_state == GameState::Busy && m_selectedUnit && !m_selectedUnit->isMoving()) {
            m_state = GameState::UnitSelected;
        }
    }

    void BattleManager::render(sf::RenderWindow& window) {
        const float VISUAL_CORRECTION = 1.31f;
        const int gridSize = 15;
        const float tileSize = 32.f;

        float midWorld = (gridSize * tileSize) / 2.0f;
        sf::Vector2f mapCenterScreen = Iso::worldToScreen({ midWorld, midWorld });
        sf::Vector2f windowSize = sf::Vector2f(window.getSize());
        sf::View cameraView(mapCenterScreen, windowSize);
        window.setView(cameraView);

        window.draw(*m_map);

        if (m_showDebug) {
            sf::VertexArray gridLines(sf::PrimitiveType::Lines);
            sf::Color gridColor(255, 255, 255, 80);

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

        if (m_state == GameState::TargetingMode && m_selectedUnit && m_pendingAbility) {
            float r = m_pendingAbility->getRange() * VISUAL_CORRECTION;
            m_rangeIndicator.setRadius(r);
            m_rangeIndicator.setScale({ 1.f, 0.5f });
            m_rangeIndicator.setOrigin({ r, r });
            m_rangeIndicator.setPosition(m_selectedUnit->getRenderPosition());
            window.draw(m_rangeIndicator);

            // Draw AOE Blast Zone
            float aoeRadius = m_pendingAbility->getRadius();
            if (aoeRadius > 0.f) {
                sf::Vector2i mousePixel = sf::Mouse::getPosition(window);
                sf::Vector2f mouseWorld = window.mapPixelToCoords(mousePixel);
                sf::Vector2f mouseLogic = Iso::screenToWorld(mouseWorld);

                float distToMouse = Iso::getDistance(m_selectedUnit->getLogicalPosition(), mouseLogic);
                if (distToMouse > m_pendingAbility->getRange()) {
                    m_aoeIndicator.setFillColor(sf::Color(255, 0, 0, 60));
                    m_aoeIndicator.setOutlineColor(sf::Color(255, 0, 0, 150));
                }
                else {
                    m_aoeIndicator.setFillColor(sf::Color(255, 165, 0, 60));
                    m_aoeIndicator.setOutlineColor(sf::Color::Yellow);
                }

                float visRadius = aoeRadius * VISUAL_CORRECTION;
                m_aoeIndicator.setRadius(visRadius);
                m_aoeIndicator.setScale({ 1.f, 0.5f });
                m_aoeIndicator.setOrigin({ visRadius, visRadius });
                m_aoeIndicator.setPosition(Iso::worldToScreen(mouseLogic));
                m_aoeIndicator.setOutlineThickness(2.f);
                window.draw(m_aoeIndicator);
            }
        }

        auto objects = m_map->getAllObjects();
        std::sort(objects.begin(), objects.end(), [](const auto& a, const auto& b) {
            return a->getRenderDepth() < b->getRenderDepth();
            });

        for (const auto& obj : objects) {
            // Draw Health Bar and Selector Ring UNDER the unit if selected
            if (auto u = std::dynamic_pointer_cast<Unit>(obj)) {
                if (u == m_selectedUnit) {
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

        if (m_state == GameState::Moving && m_selectedUnit) {
            sf::Vector2i mousePixel = sf::Mouse::getPosition(window);
            sf::Vector2f mouseWorld = window.mapPixelToCoords(mousePixel);
            sf::Vector2f mouseLogic = Iso::screenToWorld(mouseWorld);

            float maxDist = m_selectedUnit->getVitals().stamina / STAMINA_COST_PER_UNIT * VISUAL_CORRECTION;
            m_rangeIndicator.setRadius(maxDist);
            m_rangeIndicator.setScale({ 1.f, 0.5f });
            m_rangeIndicator.setOrigin({ maxDist, maxDist });
            m_rangeIndicator.setPosition(m_selectedUnit->getRenderPosition());
            m_rangeIndicator.setFillColor(sf::Color::Transparent);
            m_rangeIndicator.setOutlineColor(sf::Color(50, 255, 255, 100));
            m_rangeIndicator.setOutlineThickness(3.f);
            window.draw(m_rangeIndicator);

            float cost = 0.f;
            bool valid = isValidMove(mouseLogic, cost);
            sf::Color lineColor = valid ? sf::Color::Green : sf::Color::Red;

            // Render Multi-segment A* Path
            std::vector<sf::Vector2f> visualPath = m_map->findPath(m_selectedUnit->getLogicalPosition(), mouseLogic, m_selectedUnit.get());
            if (!visualPath.empty()) {
                sf::VertexArray pathLine(sf::PrimitiveType::LineStrip, visualPath.size() + 1);
                pathLine[0].position = m_selectedUnit->getRenderPosition();
                pathLine[0].color = lineColor;
                for (size_t i = 0; i < visualPath.size(); ++i) {
                    pathLine[i + 1].position = Iso::worldToScreen(visualPath[i]);
                    pathLine[i + 1].color = lineColor;
                }
                window.draw(pathLine);
            }
            else {
                sf::VertexArray line(sf::PrimitiveType::Lines, 2);
                line[0].position = m_selectedUnit->getRenderPosition();
                line[0].color = sf::Color::Red;
                line[1].position = mouseWorld;
                line[1].color = sf::Color::Red;
                window.draw(line);
            }

            sf::Vector2f currentWindowSize((float)window.getSize().x, (float)window.getSize().y);
            sf::View uiView(sf::FloatRect({ 0.f, 0.f }, currentWindowSize));
            window.setView(uiView);

            std::stringstream ss;
            if (valid) ss << (int)cost << " STAMINA";
            else {
                float dist = Iso::getDistance(m_selectedUnit->getLogicalPosition(), mouseLogic);
                if (dist >= 10.0f) ss << "Blocked";
            }
            m_movementTooltip.update(ss.str(), sf::Vector2f(mousePixel), windowSize);
            window.draw(m_movementTooltip);

            window.setView(cameraView);
        }

        if (m_showDebug) {
            for (const auto& obj : m_map->getAllObjects()) {
                drawDebugBox(window, obj->getCollider(), sf::Color::Magenta);
                sf::CircleShape centerDot(2.f);
                centerDot.setOrigin({ 1.f, 1.f });
                centerDot.setPosition(obj->getRenderPosition());
                centerDot.setFillColor(sf::Color::Yellow);
                window.draw(centerDot);
            }
        }

        for (const auto& ft : m_floatingTexts) window.draw(ft.text);

        if (!m_hasInitializedHUD && m_hud) {
            m_hud->onResize(window.getSize());
            m_hasInitializedHUD = true;
        }
        if (m_hud) window.draw(*m_hud);
    }

    void BattleManager::startMovementMode() {
        if (m_state == GameState::Busy || !m_selectedUnit) return;
        m_state = GameState::Moving;
    }

    bool BattleManager::isValidMove(const sf::Vector2f& target, float& outCost) {
        if (!m_selectedUnit) return false;

        float distToTarget = Iso::getDistance(m_selectedUnit->getLogicalPosition(), target);
        if (distToTarget < 10.0f) {
            outCost = 0.f;
            return false;
        }

        std::vector<sf::Vector2f> path = m_map->findPath(m_selectedUnit->getLogicalPosition(), target, m_selectedUnit.get());
        if (path.empty()) return false;

        float totalDistance = 0.f;
        sf::Vector2f currPos = m_selectedUnit->getLogicalPosition();
        for (const auto& waypoint : path) {
            totalDistance += Iso::getDistance(currPos, waypoint);
            currPos = waypoint;
        }

        outCost = totalDistance * STAMINA_COST_PER_UNIT;
        if (m_selectedUnit->getVitals().stamina < outCost) return false;
        return true;
    }

    void BattleManager::drawDebugBox(sf::RenderWindow& window, const sf::FloatRect& rect, sf::Color color) {
        sf::Vector2f p1 = { rect.position.x, rect.position.y };
        sf::Vector2f p2 = { rect.position.x + rect.size.x, rect.position.y };
        sf::Vector2f p3 = { rect.position.x + rect.size.x, rect.position.y + rect.size.y };
        sf::Vector2f p4 = { rect.position.x, rect.position.y + rect.size.y };

        sf::Vector2f s1 = Iso::worldToScreen(p1);
        sf::Vector2f s2 = Iso::worldToScreen(p2);
        sf::Vector2f s3 = Iso::worldToScreen(p3);
        sf::Vector2f s4 = Iso::worldToScreen(p4);

        sf::VertexArray lines(sf::PrimitiveType::LineStrip, 5);
        lines[0] = sf::Vertex(s1, color);
        lines[1] = sf::Vertex(s2, color);
        lines[2] = sf::Vertex(s3, color);
        lines[3] = sf::Vertex(s4, color);
        lines[4] = sf::Vertex(s1, color);
        window.draw(lines);
    }

    void BattleManager::onUnitDeath(std::shared_ptr<Unit> deadUnit) {
        std::cout << deadUnit->getName() << " has died!" << std::endl;
        m_map->removeObject(deadUnit);
        auto it = std::remove(m_turnQueue.begin(), m_turnQueue.end(), deadUnit);
        m_turnQueue.erase(it, m_turnQueue.end());
        checkBattleStatus();
    }

    void BattleManager::checkBattleStatus() {
        bool playersAlive = false;
        bool enemiesAlive = false;
        for (const auto& unit : m_turnQueue) {
            if (unit->getTeam() == Team::Player) playersAlive = true;
            if (unit->getTeam() == Team::Enemy) enemiesAlive = true;
        }

        if (!enemiesAlive) {
            std::cout << "VICTORY! All enemies defeated." << std::endl;
            m_battleOver = true;
        }
        else if (!playersAlive) {
            std::cout << "DEFEAT! Player has fallen." << std::endl;
            m_battleOver = true;
        }
    }

    void BattleManager::spawnFloatingText(sf::Vector2f location, std::string content, sf::Color color, int fontSize, sf::Vector2f velocity) {
        if (!m_font) return;
        FloatingText ft(*m_font);
        ft.text.setString(content);
        ft.text.setCharacterSize(fontSize);
        ft.text.setFillColor(color);
        ft.text.setOutlineColor(sf::Color::Black);
        ft.text.setOutlineThickness(1.5f);
        sf::FloatRect bounds = ft.text.getLocalBounds();
        ft.text.setOrigin({ bounds.size.x / 2.f, bounds.size.y / 2.f });

        ft.position = location;
        ft.position.y -= 50.f;
        ft.velocity = velocity;
        ft.lifetime = 1.2f;
        ft.maxLifetime = 1.2f;
        m_floatingTexts.push_back(ft);
    }
} 