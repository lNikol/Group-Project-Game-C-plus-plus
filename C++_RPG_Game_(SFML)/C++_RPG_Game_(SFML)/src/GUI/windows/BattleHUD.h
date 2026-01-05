#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include <memory>
#include "../widgets/StatBar.h"
#include "../widgets/Tooltip.h"
#include "../widgets/ActionSlot.h"
#include "../windows/WindowBase.h"
#include "../../interfaces/ICombatActor.h"
#include "../../core/Constants.h"
namespace RPG {

    class BattleHUD : public sf::Drawable {
    public:
        BattleHUD(const Spritesheet& iconSet, const sf::Font& font);

        void setCombatActor(ICombatActor* actor);

        void handleEvent(const sf::RenderWindow& window, const sf::Event& event);

        void onResize(const sf::Vector2u& newSize);

        void update(float dt);

    protected:
        // SFML Drawable override
        void draw(sf::RenderTarget& target, sf::RenderStates states) const override;
        std::string resolveTooltipText() const;
    private:
        // Resources
        const Spritesheet& m_iconSet;
        const sf::Font& m_font;

        // Data Source
        ICombatActor* m_actor = nullptr;

        // Vitals
        std::unique_ptr<StatBar> m_hpBar;
        std::unique_ptr<StatBar> m_mpBar;
        std::unique_ptr<StatBar> m_staminaBar;

        // Action Bar
        std::vector<std::unique_ptr<ActionSlot>> m_actionGrid;

        // System Buttons
        std::unique_ptr<ActionSlot> m_satchelBtn;
        std::unique_ptr<ActionSlot> m_journalBtn;

        // Windows
        std::unique_ptr<WindowBase> m_inventoryWindow; // Using Base Windows for now
        std::unique_ptr<WindowBase> m_journalWindow;

        // Internal State
        sf::View m_uiView;
        sf::Vector2u m_currentWindowSize;

        //Tooltip
        sf::Vector2f m_mousePos;
        mutable Tooltip m_tooltip;
    };
}