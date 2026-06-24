// ==========================================
// BattleManager.cpp
// ==========================================
#include "BattleManager.h"
#include "LootManager.h"
#include "AbilityFactory.h"
#include "PropFactory.h"
#include "UnitFactory.h"
#include "core/IsoHelpers.h"
#include "Prop.h"
#include "EndTurnCommand.h"
#include "MoveCommand.h"
#include "PartyData.h"
#include "core/Constants.h"
#include "core/EventBus.h"

#include <iostream>
#include <algorithm> 
#include <iomanip> 
#include <sstream>

namespace RPG {

    BattleManager::BattleManager(std::shared_ptr<Unit> player)
        : m_playerUnit(player),
        m_movementTooltip(*AssetManager::getInstance().getFont("PixelFont"))
    {
        m_iconSet = AssetManager::getInstance().getSpritesheet("AbilityIcons");
        m_charTexture = AssetManager::getInstance().getSpritesheet("BattleChars");
        m_monsterTexture = AssetManager::getInstance().getSpritesheet("BattleEnemies");
        m_propTexture = AssetManager::getInstance().getSpritesheet("BattleProps");
        m_tileset = AssetManager::getInstance().getSpritesheet("BattleTiles");
        m_bgTexture = AssetManager::getInstance().getSpritesheet("BattleBG");
        m_font = AssetManager::getInstance().getFont("PixelFont");

        if (!m_iconSet || !m_charTexture || !m_font) {
            std::cerr << "CRITICAL: Missing Battle Assets!" << std::endl;
        }

        m_map = std::make_unique<CombatMap>();

        if (m_iconSet && m_font) {
            m_hud = std::make_unique<BattleHUD>(*m_iconSet, *m_font);
        }
        if (m_iconSet && m_font) {
            m_hud = std::make_unique<BattleHUD>(*m_iconSet, *m_font);

            m_victoryWindow = std::make_unique<VictoryWindow>(*m_iconSet, *m_font, [this]() {
                this->m_battleResult = BattleResult::Victory; // Officially end combat
                });
            m_victoryWindow->setPosition({ Window::WIDTH / 2.0, Window::HEIGHT / 2.0 });

            m_defeatWindow = std::make_unique<DefeatWindow>(*m_font, [this]() {
                this->m_battleResult = BattleResult::Defeat; // Officially end combat
                });
            m_defeatWindow->setPosition({ Window::WIDTH / 2.0, Window::HEIGHT / 2.0 });
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

        m_aoeIndicator.setFillColor(sf::Color(255, 165, 0, 60));
        m_aoeIndicator.setOutlineColor(sf::Color::Yellow);
    }

    void BattleManager::loadEncounter(const EncounterData& data, const sf::RenderWindow& window) {
        // Reset states for a fresh battle
        m_battleResult = BattleResult::Pending;
        m_turnQueue.clear();
        m_participatingPlayers.clear();
        m_floatingTexts.clear();
        m_enemyActionPending = false;

        m_map = std::make_unique<CombatMap>();

        // 1. Setup Map Layout
        if (!data.mapInfo.backgroundTextureId.empty()) {
            const sf::Texture* bg = AssetManager::getInstance().getSpritesheet(data.mapInfo.backgroundTextureId);
            if (bg) m_map->setBackground(*bg);
        }

        if (data.mapInfo.mode == MapMode::Tiled && !data.mapInfo.tilesetId.empty()) {
            const sf::Texture* tileset = AssetManager::getInstance().getSpritesheet(data.mapInfo.tilesetId);
            if (tileset) {
                m_map->loadFromTiles(*tileset, { (unsigned)data.mapInfo.tileSize, (unsigned)data.mapInfo.tileSize },
                    data.mapInfo.gridWidth, data.mapInfo.gridHeight, data.mapInfo.tileData);
            }
        }
        else if (data.mapInfo.mode == MapMode::SingleImage && !data.mapInfo.singleImageTextureId.empty()) {
            const sf::Texture* tex = AssetManager::getInstance().getSpritesheet(data.mapInfo.singleImageTextureId);
            if (tex) m_map->loadFromImage(*tex);
        }

        // 2. Spawn Props and Enemies
        for (const auto& propSp : data.props) {
            auto prop = PropFactory::getInstance().createProp(propSp.id, propSp.logicalX, propSp.logicalY);
            if (prop) m_map->addObject(prop);
        }

        for (const auto& enSp : data.enemies) {
            auto enemy = UnitFactory::getInstance().createUnit(enSp.id, Team::Enemy, *this, enSp.logicalX, enSp.logicalY);
            if (enemy) m_map->addObject(enemy);
        }

        // 3. Setup Deployment Zone and Loot Data
        m_deploymentZone = data.deploymentZone;
        m_lootDrops = data.lootDrops;
        m_minGold = data.minGold;
        m_maxGold = data.maxGold;

        auto endTurnAction = std::make_shared<EndTurnCommand>([this]() { this->endTurn(); });
        auto moveAction = std::make_shared<MoveCommand>(*this);

        // 4. Spawn Player Party intelligently
        auto& party = PartyData::getInstance();

        for (auto& item : party.sharedInventory) {
            if (auto ability = std::dynamic_pointer_cast<CombatAbility>(item)) {
                ability->setBattleManager(this);
            }
        }

        for (auto& profile : party.activeParty) {
            if (!profile) continue;
            for (auto& item : profile->hotbar) {
                if (auto ability = std::dynamic_pointer_cast<CombatAbility>(item)) {
                    ability->setBattleManager(this);
                }
            }
            for (auto& [slot, item] : profile->equipment) {
                if (auto ability = std::dynamic_pointer_cast<CombatAbility>(item)) {
                    ability->setBattleManager(this);
                }
            }
        }
        for (const auto& profile : party.activeParty) {
            if (!profile) continue;

            sf::Vector2f spawnPos;
            bool foundPos = false;

            // Attempt A: Load preferred position
            auto prefIt = party.preferredPositions.find(profile->id);
            if (prefIt != party.preferredPositions.end()) {
                sf::Vector2f pref = prefIt->second;
                if (std::find(m_deploymentZone.begin(), m_deploymentZone.end(), pref) != m_deploymentZone.end()) {
                    if (!m_map->isBlocked(pref) && m_map->getHitObject(pref) == nullptr) {
                        spawnPos = pref;
                        foundPos = true;
                    }
                }
            }

            // Attempt B: Find first empty tile in the deployment zone
            if (!foundPos) {
                for (const auto& tilePos : m_deploymentZone) {
                    if (!m_map->isBlocked(tilePos) && m_map->getHitObject(tilePos) == nullptr) {
                        spawnPos = tilePos;
                        foundPos = true;
                        break;
                    }
                }
            }

            if (foundPos) {
                auto hero = UnitFactory::getInstance().createUnit(profile->id, Team::Player, *this, spawnPos.x, spawnPos.y);
                if (hero) {
                    // Synchronize vitals from persistent profile
                    hero->setVitals(profile->currentVitals);
                    hero->setBaseStats(profile->baseStats);
                    for (const auto& [slot, item] : profile->equipment) {
                        if (item) {
                            hero->equipItem(slot, item);
                        }
                    }
                    hero->setHotbarAbility(9, moveAction);
                    hero->setHotbarAbility(17, endTurnAction);

                    m_map->addObject(hero);
                    m_participatingPlayers.push_back(hero);

                    // Assign as primary player unit for HUD/Focus if not set
                    if (!m_playerUnit) m_playerUnit = hero;
                }
            }
            else {
                std::cerr << "Encounter Warning: No available deployment tile for " << profile->name << std::endl;
            }
        }

        if (m_hud) m_hud->onResize(window.getSize());
        m_map->onResize(window.getSize());

        // Lock into placement phase instead of starting battle immediately
        m_state = GameState::PlacementMode;
        m_selectedUnit = nullptr;
    }

    void BattleManager::finishPlacement() {
        m_selectedUnit = nullptr;
        m_state = GameState::Idle;

        auto& party = PartyData::getInstance();
        auto objects = m_map->getAllObjects();

        for (const auto& obj : objects) {
            if (auto unit = std::dynamic_pointer_cast<Unit>(obj)) {
                if (unit->getTeam() == Team::Player) {
                    party.preferredPositions[unit->getName()] = unit->getLogicalPosition();
                }
            }
        }

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

        if (m_activeUnit->getTeam() == Team::Enemy) {
            executeEnemyAI();
        }
    }

    void BattleManager::nextTurn() {
        if (m_turnQueue.empty() || m_battleResult != BattleResult::Pending) return;

        auto finishedUnit = m_turnQueue.front();
        m_turnQueue.pop_front();
        m_turnQueue.push_back(finishedUnit);

        m_activeUnit = m_turnQueue.front();

        m_activeUnit->onTurnStart(*this);

        if (m_activeUnit->isDead()) {
            std::cout << m_activeUnit->getName() << " succumbed to their wounds!" << std::endl;
            onUnitDeath(m_activeUnit);
            endTurn();
            return;
        }

        if (m_activeUnit->isStunned()) {
            std::cout << m_activeUnit->getName() << " is Stunned and loses their turn!" << std::endl;
            spawnFloatingText(m_activeUnit->getRenderPosition(), "Turn Skipped!", sf::Color::White, 20);
            endTurn();
            return;
        }

        if (m_hud) m_hud->setCombatActor(m_activeUnit.get());
        selectUnit(m_activeUnit);

        if (m_activeUnit->getTeam() == Team::Enemy) {
            executeEnemyAI();
        }
    }

    void BattleManager::endTurn() {
        if (m_state == GameState::Busy) return;
        cancelTargeting();

        if (m_activeUnit && m_battleResult == BattleResult::Pending) {
            float roll = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
            if (roll < m_activeUnit->getDoubleTurnChance()) {
                m_activeUnit->onTurnStart(*this);
                selectUnit(m_activeUnit);

                if (m_activeUnit->getTeam() == Team::Enemy) {
                    executeEnemyAI();
                }
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
        if (m_victoryWindow && m_victoryWindow->isVisible()) {
            if (m_victoryWindow->handleEvent(window, event)) return;
        }
        if (m_defeatWindow && m_defeatWindow->isVisible()) {
            if (m_defeatWindow->handleEvent(window, event)) return;
        }
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
                if (key->scancode == sf::Keyboard::Scancode::F4) m_showDebug = !m_showDebug;

                if (key->scancode == sf::Keyboard::Scancode::Enter && m_state == GameState::PlacementMode) {
                    finishPlacement();
                }
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

        // --- PLACEMENT MODE ---
        if (m_state == GameState::PlacementMode) {
            bool inDeploymentZone = false;
            sf::Vector2f snappedPos = logicalPos;
            for (const auto& zonePos : m_deploymentZone) {
                if (Iso::getDistance(zonePos, logicalPos) <= 24.0f) {
                    inDeploymentZone = true;
                    snappedPos = zonePos;
                    break;
                }
            }

            if (clickedUnit && clickedUnit->getTeam() == Team::Player) {
                if (!m_selectedUnit) {
                    m_selectedUnit = clickedUnit;
                }
                else if (m_selectedUnit != clickedUnit) {
                    sf::Vector2f tempPos = m_selectedUnit->getLogicalPosition();
                    m_selectedUnit->setLogicalPosition(clickedUnit->getLogicalPosition().x, clickedUnit->getLogicalPosition().y);
                    clickedUnit->setLogicalPosition(tempPos.x, tempPos.y);
                    m_selectedUnit = nullptr;
                }
                else {
                    m_selectedUnit = nullptr;
                }
            }
            else if (inDeploymentZone && m_selectedUnit && !clickedUnit) {
                m_selectedUnit->setLogicalPosition(snappedPos.x, snappedPos.y);
                m_selectedUnit = nullptr;
            }
            return;
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

            if (m_pendingAbility->getRadius() > 0.f) {
                auto caughtUnits = m_map->getUnitsInRadius(logicalPos, m_pendingAbility->getRadius());
                for (auto& u : caughtUnits) {
                    bool isAlly = (u->getTeam() == m_selectedUnit->getTeam());
                    if (targetType == TargetType::AreaEnemy && !isAlly) finalTargets.push_back(u);
                    else if (targetType == TargetType::AreaAlly && isAlly) finalTargets.push_back(u);
                }
            }
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
            if (m_state != GameState::PlacementMode) {
                m_state = GameState::Idle;
            }
        }
    }

    void BattleManager::selectUnit(std::shared_ptr<Unit> unit) {
        m_selectedUnit = unit;
        if (m_state != GameState::PlacementMode) {
            m_state = GameState::UnitSelected;
        }
        if (m_hud) m_hud->setCombatActor(unit.get());
    }

    void BattleManager::startTargeting(CombatAbility* ability) {
        if (!ability || !m_selectedUnit) return;

        if (ability->getTargetType() == TargetType::Self) {
            ability->resolve({ m_selectedUnit });
            m_state = GameState::UnitSelected;
            return;
        }

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
            if (m_enemyActionPending && m_selectedUnit->getTeam() == Team::Enemy) {
                m_enemyActionPending = false;
                m_state = GameState::UnitSelected;
                executeEnemyAI();
            }
            else {
                m_state = GameState::UnitSelected;
            }
        }
        checkDeaths();
    }
    void BattleManager::checkDeaths() {
        std::vector<std::shared_ptr<Unit>> deadUnits;
        for (const auto& unit : m_turnQueue) {
            if (unit->isDead()) {
                deadUnits.push_back(unit);
            }
        }

        for (const auto& dead : deadUnits) {
            onUnitDeath(dead);
            if (m_activeUnit == dead) {
                m_activeUnit = nullptr;
                nextTurn();
            }
        }
    }
    void BattleManager::render(sf::RenderWindow& window) {
        const float VISUAL_CORRECTION = 1.41f;
        const int gridSize = 15;
        const float tileSize = 32.f;

        float midWorld = (gridSize * tileSize) / 2.0f;
        sf::Vector2f mapCenterScreen = Iso::worldToScreen({ midWorld, midWorld });
        sf::Vector2f windowSize = sf::Vector2f(window.getSize());
        sf::View cameraView(mapCenterScreen, Window::getLogicalSize(window.getSize()));
        window.setView(cameraView);

        window.draw(*m_map);

        // Draw Deployment Zone if in Placement Mode
        if (m_state == GameState::PlacementMode) {
            sf::ConvexShape diamond;
            diamond.setPointCount(4);
            float s = tileSize;

            diamond.setPoint(0, Iso::worldToScreen({ 0.f, s / 2.f }));
            diamond.setPoint(1, Iso::worldToScreen({ s / 2.f, 0.f }));
            diamond.setPoint(2, Iso::worldToScreen({ s, s / 2.f }));
            diamond.setPoint(3, Iso::worldToScreen({ s / 2.f, s }));

            diamond.setFillColor(sf::Color(0, 100, 255, 80));
            diamond.setOutlineColor(sf::Color(100, 200, 255, 200));
            diamond.setOutlineThickness(2.f);

            for (const auto& logicPos : m_deploymentZone) {
                sf::Vector2f tileTopLeft(logicPos.x - (s / 2.f), logicPos.y - (s / 2.f));
                diamond.setPosition(Iso::worldToScreen(tileTopLeft));
                window.draw(diamond);
            }
        }

        if (m_showDebug) {
            sf::VertexArray grid(sf::PrimitiveType::Lines);
            sf::Color gridColor(255, 255, 255, 80);

            for (int x = 0; x <= gridSize; ++x) {
                sf::Vector2f start = Iso::worldToScreen({ x * tileSize, 0.f });
                sf::Vector2f end = Iso::worldToScreen({ x * tileSize, gridSize * tileSize });
                grid.append(sf::Vertex(start, gridColor));
                grid.append(sf::Vertex(end, gridColor));
            }
            for (int y = 0; y <= gridSize; ++y) {
                sf::Vector2f start = Iso::worldToScreen({ 0.f, y * tileSize });
                sf::Vector2f end = Iso::worldToScreen({ gridSize * tileSize, y * tileSize });
                grid.append(sf::Vertex(start, gridColor));
                grid.append(sf::Vertex(end, gridColor));
            }

            window.draw(grid);
        }

        if (m_state == GameState::TargetingMode && m_selectedUnit && m_pendingAbility) {
            float r = m_pendingAbility->getRange() * VISUAL_CORRECTION;
            m_rangeIndicator.setRadius(r);
            m_rangeIndicator.setScale({ 1.f, 0.5f });
            m_rangeIndicator.setOrigin({ r, r });
            m_rangeIndicator.setPosition(m_selectedUnit->getRenderPosition());
            window.draw(m_rangeIndicator);

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
            if (auto u = std::dynamic_pointer_cast<Unit>(obj)) {
                if (u == m_selectedUnit) {
                    m_selector.setPosition(obj->getRenderPosition());
                    window.draw(m_selector);
                }

                const auto* sprite = obj->getSprite();
                float hp = u->getVitals().hp;
                float maxHp = u->getVitals().maxHp;
                float ratio = hp / maxHp;

                sf::Vector2f pos = sprite->getPosition();

                float barWidth = 40.f;
                float barHeight = 5.f;
                float offsetY = -70.f;

                sf::RectangleShape bg;
                bg.setSize({ barWidth, barHeight });
                bg.setFillColor(sf::Color::Black);
                bg.setOrigin({ barWidth / 2.f, barHeight / 2.f });
                bg.setPosition({ pos.x, pos.y + offsetY });

                sf::RectangleShape hpBar;
                hpBar.setSize({ barWidth * ratio, barHeight });
                hpBar.setFillColor(sf::Color::Red);

                hpBar.setOrigin({ 0.f, barHeight / 2.f });
                hpBar.setPosition({ pos.x - (barWidth / 2.f),  pos.y + offsetY });

                window.draw(bg);
                window.draw(hpBar);
            }
            obj->updateVisuals();

            if (const auto* sprite = obj->getSprite()) {
                window.draw(*sprite);
            }
            else {
                sf::CircleShape debug(10.f);
                debug.setPosition(obj->getRenderPosition());
                window.draw(debug);
            }
        }

        if (m_state != GameState::PlacementMode) {
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

                sf::Color color = sf::Color::Green;
                if (unit->getTeam() == Team::Enemy)
                    color = sf::Color::Red;
                if (i == 0)
                {
                    sf::ConvexShape arrow;
                    arrow.setPointCount(3);
                    arrow.setPoint(0, sf::Vector2f(0.f, 0.f));
                    arrow.setPoint(1, sf::Vector2f(20.f, 0.f));
                    arrow.setPoint(2, sf::Vector2f(10.f, -20.f));
                    arrow.setFillColor(sf::Color::Yellow);
                    arrow.setPosition(sf::Vector2f(startX + i * spacing + 10.f, startY + 70.f));
                    window.draw(arrow);
                }

                sf::CircleShape highlight;
                highlight.setRadius(25.f);
                highlight.setFillColor(sf::Color::Transparent);
                highlight.setOutlineColor(color);
                highlight.setOutlineThickness(3.f);
                highlight.setOrigin({ 25.f, 25.f });
                highlight.setPosition({ startX + i * spacing + 21.f, startY + 21.f });

                window.draw(highlight);
                window.draw(icon);
            }
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
            sf::View uiView(sf::FloatRect({ 0.f, 0.f }, Window::getLogicalSize(window.getSize())));
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
                if (!obj) continue;
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
        if (m_hud && m_state != GameState::PlacementMode) window.draw(*m_hud);
        if (m_victoryWindow && m_victoryWindow->isVisible()) {
            window.setView(window.getDefaultView());
            window.draw(*m_victoryWindow);
        }
        if (m_defeatWindow && m_defeatWindow->isVisible()) {
            window.setView(window.getDefaultView());
            window.draw(*m_defeatWindow);
        }
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
        sf::VertexArray lines(sf::PrimitiveType::LineStrip, 5);

        lines[0].position = Iso::worldToScreen({ rect.position.x, rect.position.y });
        lines[0].color = color;
        lines[1].position = Iso::worldToScreen({ rect.position.x + rect.size.x, rect.position.y });
        lines[1].color = color;
        lines[2].position = Iso::worldToScreen({ rect.position.x + rect.size.x, rect.position.y + rect.size.y });
        lines[2].color = color;
        lines[3].position = Iso::worldToScreen({ rect.position.x, rect.position.y + rect.size.y });
        lines[3].color = color;
        lines[4].position = lines[0].position;
        lines[4].color = color;

        window.draw(lines);
    }

    void BattleManager::onUnitDeath(std::shared_ptr<Unit> deadUnit) {
        std::cout << deadUnit->getName() << " has died!" << std::endl;

        if (deadUnit->getTeam() == Team::Enemy) {
            EventBus::getInstance().publish(EnemyDefeatedEvent(deadUnit->getId()));
        }

        m_map->removeObject(deadUnit);
        auto it = std::remove(m_turnQueue.begin(), m_turnQueue.end(), deadUnit);
        m_turnQueue.erase(it, m_turnQueue.end());
        checkBattleStatus();
    }

    void BattleManager::executeEnemyAI() {
        if (!m_activeUnit || m_activeUnit->getTeam() != Team::Enemy) return;

        std::shared_ptr<Unit> closestPlayer = nullptr;
        float minDist = 99999.f;
        for (const auto& unit : m_turnQueue) {
            if (unit->getTeam() == Team::Player && !unit->isDead()) {
                float dist = Iso::getDistance(m_activeUnit->getLogicalPosition(), unit->getLogicalPosition());
                if (dist < minDist) {
                    minDist = dist;
                    closestPlayer = unit;
                }
            }
        }

        if (!closestPlayer) {
            endTurn();
            return;
        }

        std::shared_ptr<CombatAbility> chosenAbility = nullptr;
        for (uint8_t i = 0; i < m_activeUnit->getHotbarSize(); ++i) {
            auto ability = std::dynamic_pointer_cast<CombatAbility>(m_activeUnit->getHotbarAbility(i));
            if (ability && ability->canBeCast() && ability->getTargetType() != TargetType::Self) {
                chosenAbility = ability;
                break;
            }
        }

        if (!chosenAbility) {
            endTurn();
            return;
        }

        if (minDist <= chosenAbility->getRange()) {
            chosenAbility->setOwner(m_activeUnit.get());
            chosenAbility->resolve({ closestPlayer });
            endTurn();
        }
        else {
            sf::Vector2f targetPos = closestPlayer->getLogicalPosition();
            sf::Vector2f bestDest = targetPos;
            float bestDist = 99999.f;

            std::vector<sf::Vector2f> offsets = { {35.f, 0.f}, {-35.f, 0.f}, {0.f, 35.f}, {0.f, -35.f} };
            for (auto offset : offsets) {
                sf::Vector2f testPos = targetPos + offset;
                if (!m_map->isBlocked(testPos)) {
                    float d = Iso::getDistance(m_activeUnit->getLogicalPosition(), testPos);
                    if (d < bestDist) {
                        bestDist = d;
                        bestDest = testPos;
                    }
                }
            }
            std::vector<sf::Vector2f> fullPath = m_map->findPath(m_activeUnit->getLogicalPosition(), bestDest, m_activeUnit.get());
            if (!fullPath.empty()) {
                float currentStamina = m_activeUnit->getVitals().stamina;
                float cost = 0.f;
                std::vector<sf::Vector2f> truncatedPath;
                sf::Vector2f currPos = m_activeUnit->getLogicalPosition();

                for (const auto& wp : fullPath) {
                    float stepCost = Iso::getDistance(currPos, wp) * STAMINA_COST_PER_UNIT;
                    if (cost + stepCost > currentStamina) break;
                    cost += stepCost;
                    truncatedPath.push_back(wp);
                    currPos = wp;
                }

                if (!truncatedPath.empty()) {
                    m_activeUnit->consumeStamina(cost);
                    m_activeUnit->setPath(truncatedPath);
                    m_state = GameState::Busy;
                    m_enemyActionPending = true;
                }
                else {
                    endTurn();
                }
            }
            else {
                endTurn();
            }
        }
    }

    void BattleManager::checkBattleStatus() {
        // Prevent triggering twice if windows are already open
        if (m_battleResult != BattleResult::Pending ||
            (m_victoryWindow && m_victoryWindow->isVisible()) ||
            (m_defeatWindow && m_defeatWindow->isVisible())) return;

        bool playersAlive = false;
        bool enemiesAlive = false;
        for (const auto& unit : m_turnQueue) {
            if (unit->getTeam() == Team::Player && !unit->isDead()) playersAlive = true;
            if (unit->getTeam() == Team::Enemy && !unit->isDead()) enemiesAlive = true;
        }

        if (!enemiesAlive) {
            m_state = GameState::Busy; // Lock UI interactions
            finalizeBattle(BattleResult::Victory);
        }
        else if (!playersAlive) {
            m_state = GameState::Busy;
            finalizeBattle(BattleResult::Defeat);
        }
    }

    void BattleManager::finalizeBattle(BattleResult result) {
        auto& party = PartyData::getInstance();

        for (auto& unit : m_participatingPlayers) {
            for (auto& profile : party.activeParty) {
                if (profile && profile->id == unit->getName()) {
                    profile->currentVitals = unit->getVitals();
                    if (result == BattleResult::Defeat && profile->currentVitals.hp <= 0) {
                        profile->currentVitals.hp = 1.0f;
                    }
                }
            }
        }

        if (result == BattleResult::Victory) {
            int goldReward = LootManager::rollGold(m_minGold, m_maxGold);
            party.gold += goldReward;

            auto rolledItems = LootManager::rollLoot(m_lootDrops, *this);
            std::vector<std::shared_ptr<IAbility>> lootedItems;

            for (auto& item : rolledItems) {
                if (party.addItem(item)) {
                    lootedItems.push_back(item);
                }
            }

            if (m_victoryWindow) m_victoryWindow->setResults(goldReward, lootedItems);
        }
        else {
            int penalty = std::min(party.gold, 20);
            party.gold -= penalty;
            if (m_defeatWindow) m_defeatWindow->setResults(penalty);
        }
        party.clearConsumedItems();
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

    BattleManager::~BattleManager() {
        auto& party = PartyData::getInstance();

        auto cleanPointers = [](std::shared_ptr<IAbility> ability) {
            if (auto combatAbility = std::dynamic_pointer_cast<CombatAbility>(ability)) {
                combatAbility->setOwner(nullptr);
                combatAbility->setBattleManager(nullptr);
            }
            };

        for (auto& item : party.sharedInventory) {
            if (item) cleanPointers(item);
        }

        for (auto& profile : party.activeParty) {
            if (!profile) continue;

            for (auto& ability : profile->hotbar) {
                if (ability) cleanPointers(ability);
            }
            for (auto& [slot, equipment] : profile->equipment) {
                if (equipment) cleanPointers(equipment);
            }
        }
    }
}