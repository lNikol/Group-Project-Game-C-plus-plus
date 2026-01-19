#pragma once
#include <unordered_map>
#include <string>
#include <iostream>
#include <SFML/Graphics.hpp>
#include "Constants.h"

namespace RPG {

    /**
     * @brief Centralized manager for loading and retrieving game assets.
     * * The AssetManager handles the lifespan of Spritesheets, Textures, and Fonts.
     * It ensures resources are loaded from files only once and accessed via
     * string identifiers.
     */
    class AssetManager {
    private:
        // Storage
        std::unordered_map<std::string, Spritesheet> spritesheetMap;
        std::unordered_map<std::string, sf::Texture> textureMap;
        std::unordered_map<std::string, sf::Font> fontMap;

    public:
        // ==============================
        // Spritesheets
        // ==============================

        /**
         * @brief Loads a Spritesheet from a file and stores it under a unique name.
         * * If a spritesheet with the given name already exists, or if the file
         * fails to load, an error is printed to std::cerr and the operation is aborted.
         * * @param name The unique identifier for the spritesheet.
         * @param filepath The path to the image file to load.
         */
        void addSpritesheet(const std::string& name, const std::string& filepath);

        /**
         * @brief Retrieves a pointer to a stored Spritesheet.
         * * @param name The unique identifier used when adding the spritesheet.
         * @return const Spritesheet* Pointer to the spritesheet, or nullptr if not found.
         */
        const Spritesheet* getSpritesheet(const std::string& name) const;


        // ==============================
        // Textures
        // ==============================

        /**
         * @brief Loads an sf::Texture from a file and stores it.
         * * Used for standalone images (backgrounds, UI elements) that are not
         * part of a spritesheet animation.
         * * @param name The unique identifier for the texture.
         * @param filepath The path to the image file.
         */
        void addTexture(const std::string& name, const std::string& filepath);

        /**
         * @brief Retrieves a pointer to a stored sf::Texture.
         * * @param name The unique identifier for the texture.
         * @return const sf::Texture* Pointer to the texture, or nullptr if not found.
         */
        const sf::Texture* getTexture(const std::string& name) const;


        // ==============================
        // Fonts
        // ==============================

        /**
         * @brief Loads an sf::Font from a file and stores it.
         * * @param name The unique identifier for the font.
         * @param filepath The path to the font file (e.g., .ttf).
         */
        void addFont(const std::string& name, const std::string& filepath);

        /**
         * @brief Retrieves a pointer to a stored sf::Font.
         * * @param name The unique identifier for the font.
         * @return const sf::Font* Pointer to the font, or nullptr if not found.
         */
        const sf::Font* getFont(const std::string& name) const;
    };

}