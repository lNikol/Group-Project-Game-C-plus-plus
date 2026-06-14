// ==========================================
// BattleManager.h
// ==========================================
#pragma once
#include <SFML/Graphics.hpp>
#include <memory>
#include <vector>
#include <deque>

#include "CombatMap.h"
#include "Unit.h"
#include "GUI/windows/BattleHUD.h"
#include "GUI/widgets/Tooltip.h"
#include "CombatAbility.h"
#include "worldmap/AssetManager.h"
#include "EncounterLoader.h"

namespace RPG {

    /**
     * @brief Represents a temporary text label floating in the world.
     */
    struct FloatingText {
        sf::Text text;
        sf::Vector2f position;
        sf::Vector2f velocity;
        float lifetime;
        float maxLifetime;
        FloatingText(const sf::Font& font) : text(font) {}
    };

    enum class GameState {
        PlacementMode,  ///< Phase 1: Player places units on the grid
        Idle,           ///< Phase 2: Standard combat loop waiting for input
        UnitSelected,
        TargetingMode,
        Moving,
        Busy
    };

    enum class BattleResult {
        Pending,
        Victory,
        Defeat
    };

    class BattleManager {
    public:
        /**
         * @brief Initializes the combat controller.
         * @param player The persistent player object from the GameManager.
         */
        BattleManager(std::shared_ptr<Unit> player);
        ~BattleManager() = default;

        /**
         * @brief Loads the map layout, props, and enemies from parsed EncounterData.
         * Initiates the PlacementMode for the player's active party.
         * @param data The parsed JSON data containing the encounter layout.
         * @param window Reference to the render window for UI scaling.
         */
        void loadEncounter(const EncounterData& data, const sf::RenderWindow& window);

        /**
         * @brief Locks in the player's placement, saves preferences, and starts the turn queue.
         */
        void finishPlacement();

        void spawnFloatingText(sf::Vector2f location, std::string content, sf::Color color, int fontSize = 20, sf::Vector2f velocity = { 0.f, -50.f });

        // ==============================
        // Game Loop
        // ==============================
        void handleEvent(sf::RenderWindow& window, const sf::Event& event);
        void update(float dt);
        void render(sf::RenderWindow& window);

        // ==============================
        // Ability Interface
        // ==============================
        void startTargeting(CombatAbility* ability);
        void cancelTargeting();

        // Turn control
        void startBattle();
        void nextTurn();
        void endTurn();
        bool isPlayerTurn() const;
        void startMovementMode();
        void checkDeaths();
        void onUnitDeath(std::shared_ptr<Unit> deadUnit);

        bool isBattleOver() const { return m_battleResult != BattleResult::Pending; }
        BattleResult getBattleResult() const { return m_battleResult; }

        BattleHUD* getHUD() const { return m_hud.get(); }

        /**
         * @brief Returns the unit that is currently taking its turn.
         * @return std::shared_ptr<Unit> The active combatant.
         */
        std::shared_ptr<Unit> getActiveUnit() const { return m_activeUnit; }

    private:
        std::unique_ptr<CombatMap> m_map;
        std::unique_ptr<BattleHUD> m_hud;
        std::shared_ptr<Unit> m_playerUnit;

        GameState m_state = GameState::Idle;
        BattleResult m_battleResult = BattleResult::Pending;
        std::shared_ptr<Unit> m_selectedUnit;
        CombatAbility* m_pendingAbility = nullptr;

        sf::CircleShape m_selector;
        sf::CircleShape m_rangeIndicator;
        sf::CircleShape m_aoeIndicator;

        std::shared_ptr<Unit> m_activeUnit;
        std::deque<std::shared_ptr<Unit>> m_turnQueue;
        std::vector<std::shared_ptr<Unit>> m_participatingPlayers;

        std::vector<sf::Vector2f> m_deploymentZone;
        std::vector<std::string> m_lootItemIds;
        bool m_enemyActionPending = false;

        bool m_hasInitializedHUD = false;
        const sf::Texture* m_iconSet = nullptr;
        const sf::Texture* m_charTexture = nullptr;
        const sf::Texture* m_monsterTexture = nullptr;
        const sf::Texture* m_propTexture = nullptr;
        const sf::Texture* m_tileset = nullptr;
        const sf::Texture* m_bgTexture = nullptr;
        const sf::Font* m_font = nullptr;

        Tooltip m_movementTooltip;
        const float STAMINA_COST_PER_UNIT = 0.5f;

        bool m_showDebug = false;
        std::vector<FloatingText> m_floatingTexts;

        // ==============================
        // Internal Mechanics
        // ==============================
        void onLeftClick(const sf::RenderWindow& window, const sf::Vector2i& mousePos);
        void onRightClick();
        void selectUnit(std::shared_ptr<Unit> unit);
        bool isValidMove(const sf::Vector2f& target, float& outCost);
        void drawDebugBox(sf::RenderWindow& window, const sf::FloatRect& rect, sf::Color color);
        void checkBattleStatus();

        /**
         * @brief Evaluates the battlefield and executes the AI logic for the active enemy.
         */
        void executeEnemyAI();

        /**
         * @brief Synchronizes Unit vitals back to PartyData and applies loot/penalties.
         */
        void finalizeBattle();
    };
}