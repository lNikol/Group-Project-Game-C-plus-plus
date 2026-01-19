#pragma once
#include "Interfaces/IAbility.h"
#include <functional>
#include <string>

namespace RPG {

    /**
     * @brief A special "Ability" implementation used for UI interactions.
     * * This allows UI buttons (like the Inventory Satchel or Journal) to be treated
     * identically to combat skills (ActionSlots).
     * * Instead of casting a spell, executing this command triggers a std::function callback.
     */
    class OpenWindowCommand : public IAbility {
    public:
        /**
         * @brief Constructs the command.
         * @param iconIndex The sprite index for the button icon.
         * @param tooltip The text to display on hover.
         * @param onExecute The lambda/function to run when clicked (e.g., toggling a window).
         */
        OpenWindowCommand(int iconIndex, std::string tooltip, std::function<void()> onExecute)
            : m_iconIndex(iconIndex), m_tooltip(tooltip), m_onExecute(onExecute) {
        }

        // Visuals
        int getIconIndex() const override { return m_iconIndex; }
        std::string getTooltip() const override { return m_tooltip; }

        // Mechanics
        bool canBeCast() const override { return m_enabled; }

        /**
         * @brief Always returns -1 as UI buttons typically don't run out of charges.
         */
        int getCharges() const override { return -1; }

        /**
         * @brief Always returns 0 as UI buttons (usually) don't have cooldowns.
         */
        int getCooldown() const override { return 0; }

        // Execution
        void execute() override {
            if (m_onExecute) {
                m_onExecute();
            }
        }

        /**
         * @brief Manually enable or disable the button (e.g., during cutscenes).
         */
        void setEnabled(bool enabled) { m_enabled = enabled; }

    private:
        int m_iconIndex;
        std::string m_tooltip;
        std::function<void()> m_onExecute;
        bool m_enabled = true;
    };
}