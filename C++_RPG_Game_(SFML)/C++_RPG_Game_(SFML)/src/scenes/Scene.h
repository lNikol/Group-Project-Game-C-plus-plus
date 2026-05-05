#pragma once
#include <SFML/Graphics.hpp>
#include "../worldmap/AssetManager.h"

namespace RPG {

    /**
     * @brief Abstract base class for all game states/screens.
     * * The Scene interface defines the mandatory lifecycle methods for any part of the game
     * (e.g., Menu, World Map, Battle). It allows the SceneController to manage transitions
     * and execution of different game logic modules polymorphically.
     */
    class Scene {
    public:
        virtual ~Scene() = default;
        

        /**
         * @brief Processes input events (mouse clicks, movement, etc.).
         * * @param window The window reference (used for coordinate mapping).
         * @param event The specific SFML event to process.
         * @return true if the event was consumed by this element (stops propagation), false otherwise.
         */
        virtual bool handleEvent(sf::RenderWindow& window, const sf::Event& event) {
            return false;
        }

        /**
         * @brief Updates scene logic and state.
         * @param dt Delta time in seconds since the last frame.
         * @param window Reference to the render window for coordinate mapping.
         */
        virtual void update(float dt) = 0;

        /**
         * @brief Renders the scene's visual components to the window.
         * @param window The target SFML RenderWindow.
         */
        virtual void draw(sf::RenderWindow& window) = 0;
    };
}