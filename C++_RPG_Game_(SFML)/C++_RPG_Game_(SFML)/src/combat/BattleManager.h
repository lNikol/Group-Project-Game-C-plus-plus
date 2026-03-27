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
    /** * @brief Represents a temporary text label floating in the world.
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
        Idle,
        UnitSelected,
        TargetingMode,
        Moving,
        Busy
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
        BattleManager(std::shared_ptr<Unit> player);
        ~BattleManager() = default;

        void initTestLevel(const sf::RenderWindow& window);
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
        void onUnitDeath(std::shared_ptr<Unit> deadUnit);
        bool isBattleOver() { return m_battleOver; }

    private:
        std::unique_ptr<CombatMap> m_map;
        std::unique_ptr<BattleHUD> m_hud;
        std::shared_ptr<Unit> m_playerUnit;

        GameState m_state = GameState::Idle;
        std::shared_ptr<Unit> m_selectedUnit;
        CombatAbility* m_pendingAbility = nullptr;

        sf::CircleShape m_selector;
        sf::CircleShape m_rangeIndicator;
        sf::CircleShape m_aoeIndicator; // NEW: Visualizer for AOE blasts

        void onLeftClick(const sf::RenderWindow& window, const sf::Vector2i& mousePos);
        void onRightClick();
        void selectUnit(std::shared_ptr<Unit> unit);

        std::shared_ptr<Unit> m_activeUnit;
        std::deque<std::shared_ptr<Unit>> m_turnQueue;

        bool m_hasInitializedHUD = false;
        //character textures
        const sf::Texture* m_iconSet = nullptr;
        const sf::Texture* m_charTexture = nullptr;
        const sf::Texture* m_monsterTexture = nullptr;
        const sf::Texture* m_propTexture = nullptr;
        const sf::Texture* m_tileset = nullptr;
        const sf::Texture* m_bgTexture = nullptr;
        const sf::Font* m_font = nullptr;

        bool isValidMove(const sf::Vector2f& target, float& outCost);

        Tooltip m_movementTooltip;
        const float STAMINA_COST_PER_UNIT = 0.5f;

        bool m_showDebug = false;
        std::vector<FloatingText> m_floatingTexts;
        void drawDebugBox(sf::RenderWindow& window, const sf::FloatRect& rect, sf::Color color);
        void checkBattleStatus();
        bool m_battleOver = false;
    };
}