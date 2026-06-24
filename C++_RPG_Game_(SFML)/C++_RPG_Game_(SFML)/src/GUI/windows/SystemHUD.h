#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include <memory>
#include "GUI/widgets/ActionSlot.h"
#include "GUI/windows/WindowBase.h"
#include "GUI/widgets/Tooltip.h"

namespace RPG {

    class SystemHUD : public sf::Drawable {
    public:
        SystemHUD(const Spritesheet& iconSet, const sf::Font& font);
        ~SystemHUD() = default;

        void handleEvent(sf::RenderWindow& window, const sf::Event& event, bool isBattleScene);
        void onResize(const sf::Vector2u& newSize);
        void update(float dt);

        bool isMouseOverUI() const;
        void closeAllWindows();
        
        // Quest feedback
        void notifyQuestUpdated();

    protected:
        void draw(sf::RenderTarget& target, sf::RenderStates states) const override;
        std::string resolveTooltipText() const;

    private:
        const Spritesheet& m_iconSet;
        const sf::Font& m_font;

        // System Buttons
        std::unique_ptr<ActionSlot> m_satchelBtn;
        std::unique_ptr<ActionSlot> m_journalBtn;
        std::unique_ptr<ActionSlot> m_bestiaryBtn;

        // Windows
        std::unique_ptr<WindowBase> m_inventoryWindow;
        std::unique_ptr<WindowBase> m_journalWindow;
        std::unique_ptr<WindowBase> m_bestiaryWindow;

        // Internal State
        sf::View m_uiView;
        sf::Vector2u m_currentWindowSize;

        sf::Vector2f m_mousePos;
        mutable Tooltip m_tooltip;
        
        // Visual Feedback
        float m_questHighlightTimer = 0.0f;
    };
}
