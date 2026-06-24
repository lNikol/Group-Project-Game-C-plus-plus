#pragma once
#include "GUI/windows/WindowBase.h"
#include "GUI/widgets/ActionSlot.h"
#include "GUI/widgets/TextWidget.h"
#include "Interfaces/IAbility.h"
#include "core/Constants.h"
#include <vector>
#include <memory>
#include <functional>

namespace RPG {
    class VictoryWindow : public WindowBase {
    public:
        /**
         * @param onCloseCallback Triggers when the 'X' button is clicked, allowing BattleManager to safely exit.
         */
        VictoryWindow(const Spritesheet& iconSet, const sf::Font& font, std::function<void()> onCloseCallback);

        /**
         * @brief Parses the rewards, builds the layout, and shows the window.
         */
        void setResults(int goldReward, const std::vector<std::shared_ptr<IAbility>>& lootedItems);

        bool handleEvent(sf::RenderWindow& window, const sf::Event& event) override;

    private:
        const Spritesheet& m_iconSet;
        const sf::Font& m_font;
        std::function<void()> m_onCloseCallback;

        void buildLayout(int goldReward, const std::vector<std::shared_ptr<IAbility>>& lootedItems);
    };
}