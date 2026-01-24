#pragma once
#include <SFML/Graphics.hpp>
#include <memory>
#include <vector>

#include "CombatMap.h"
#include "Unit.h"
#include "GUI/windows/BattleHUD.h"
#include "GUI/widgets/Tooltip.h"
#include "CombatAbility.h"
#include <deque>
#include"worldmap/AssetManager.h"
namespace RPG {

    enum class GameState {
        Idle,           ///< Free selection mode.
        UnitSelected,   ///< A unit is selected, HUD is active.
        TargetingMode,  ///< Player clicked a skill, waiting for map target.
        Moving,
        Busy            ///< Animations playing, input blocked.
    };

    class BattleManager {
    public:
        /**
         * @brief Initializes the combat controller.
         * @param iconSet The texture for ability icons (passed to HUD).
         * @param charTexture the texture for units.
         * @param font The font for UI text (passed to HUD).
         * @param player The persistent player object from the GameManager.
         */
        BattleManager(const AssetManager& assetManager,
            std::shared_ptr<Unit> player);
        ~BattleManager() = default;

        /**
         * @brief Sets up the battlefield (Spawns enemies, walls, places Player).
         */
        void initTestLevel();

        // ==============================
        // Game Loop
        // ==============================

        /**
         * @brief Handles input with UI Blocking priority.
         * * 1. Passes event to HUD using the Window for coordinate mapping.
         * * 2. If HUD did NOT hover/consume, passes event to Map Logic.
         * @param window Required for mapping mouse pixels to world coordinates.
         * @param event The input event to process.
         */
        void handleEvent(sf::RenderWindow& window, const sf::Event& event);

        void update(float dt);

        /**
         * @brief Renders the Map (Bottom) then the HUD (Top).
         */
        void render(sf::RenderWindow& window);

        // ==============================
        // Ability Interface
        // ==============================

        /**
         * @brief Switch to targeting mode for a specific ability.
         * * Called by CombatAbility::execute().
         */
        void startTargeting(CombatAbility* ability);
        void cancelTargeting();


        //turn control
        void startBattle();
        void nextTurn();
        void endTurn();
        // Check if it is currently the local player's turn (controls UI locking)
        bool isPlayerTurn() const;

        void startMovementMode();

        void onUnitDeath(std::shared_ptr<Unit> deadUnit);
        bool isBattleOver() { return m_battleOver; }
    private:
        // Core Components
        std::unique_ptr<CombatMap> m_map;
        std::unique_ptr<BattleHUD> m_hud;

        // Persistent Data
        std::shared_ptr<Unit> m_playerUnit;

        // State Machine
        GameState m_state = GameState::Idle;
        std::shared_ptr<Unit> m_selectedUnit;      // Unit currently being inspected
        CombatAbility* m_pendingAbility = nullptr; // Ability waiting for target

        // Visuals
        sf::CircleShape m_selector;
        sf::CircleShape m_rangeIndicator;

        // Internal Helpers
        void onLeftClick(const sf::RenderWindow& window, const sf::Vector2i& mousePos);
        void onRightClick();
        void selectUnit(std::shared_ptr<Unit> unit);

        // The Active Unit is the one currently allowed to move/act
        std::shared_ptr<Unit> m_activeUnit;

        // The Turn Order
        std::deque<std::shared_ptr<Unit>> m_turnQueue;
        //HUD
        bool m_hasInitializedHUD = false;
        //assets
        const AssetManager& m_assetManager;
        //character textures
        const sf::Texture* m_iconSet = nullptr;
        const sf::Texture* m_charTexture = nullptr;
        const sf::Texture* m_monsterTexture = nullptr;
        const sf::Texture* m_propTexture = nullptr;
        const sf::Texture* m_tileset = nullptr;
        const sf::Texture* m_bgTexture = nullptr;
        const sf::Font* m_font = nullptr;

        // Helpers for movement
        bool isValidMove(const sf::Vector2f& target, float& outCost);

        // Visuals for movement
        Tooltip m_movementTooltip; // For showing "Cost: 15"
        const float STAMINA_COST_PER_UNIT = 0.5f; // 1 pixel = 0.5 stamina

        // --- NEW DEBUG FLAG ---
        bool m_showDebug = false;

        // Helper to draw a projected box
        void drawDebugBox(sf::RenderWindow& window, const sf::FloatRect& rect, sf::Color color);

        // Helper to check win conditions
        void checkBattleStatus();

        bool m_battleOver = false;
    };
}