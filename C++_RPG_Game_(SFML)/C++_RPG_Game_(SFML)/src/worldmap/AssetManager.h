#pragma once
#include <unordered_map>
#include <string>
#include <iostream>
#include <filesystem>
#include <SFML/Graphics.hpp>
#include "StructureDefinition.h"
#include "core/SpritesheetManager.h"
#include "core/GameConfig.h"

namespace RPG {


    struct AssetDefinition {
        std::string name;
        int x, y, w, h;          // Pozycja w atlas.png
        int hb_w, hb_h;          // Wymiary hitboxa
        int off_x, off_y;        // Offset hitboxa wzgl?dem punktu (x,y)
        bool blocks;             // Czy obiekt jest przeszkod?
        StructureType type;      // Typ enuma rozpoznany po nazwie
    };

    /**
     * @brief Centralized manager for game assets.
     *
     * AssetManager handles both:
     * 1. Structure definitions (`structureLibrary`)
     * 2. Spritesheets, textures, and fonts
     *
     * It ensures resources are loaded only once and provides
     * easy access via string identifiers or structure types.
     */
    class AssetManager {
    private:
        // ==============================
        // Structure Definitions
        // ==============================
        std::unordered_map<std::string, StructureDefinition> structureLibrary;

        // ==============================
        // Spritesheets, Textures, Fonts
        // ==============================
        std::unordered_map<std::string, Spritesheet> spritesheetMap;
        std::unordered_map<std::string, sf::Texture> textureMap;
        std::unordered_map<std::string, sf::Font> fontMap;

    public:
        AssetManager() = default;

        // ==============================
        // Structure Management
        // ==============================

        /**
         * @brief Initializes structure definitions and registers necessary textures.
         * @param sm Reference to a SpritesheetManager for registering textures.
         *
         * This function populates `structureLibrary` with default terrain,
         * obstacles, and special zones, and loads required textures into
         * the spritesheet manager.
         */
        void init();

        /**
         * @brief Retrieves the definition for a specific structure type.
         * @param type The structure type to query.
         * @return const StructureDefinition& Reference to the structure definition.
         *
         * If the type is not found, it returns the definition for `StructureType::None`.
         */
        const StructureDefinition& getDefinition(const std::string& name) const;

        // ==============================
        // Spritesheets
        // ==============================

        /**
         * @brief Loads a spritesheet from file and stores it under a unique name.
         * @param name The unique identifier for the spritesheet.
         * @param filepath Path to the image file.
         *
         * Prints an error if the name already exists or if loading fails.
         */
        void addSpritesheet(const std::string& name, const std::string& filepath);

        /**
         * @brief Retrieves a pointer to a stored spritesheet.
         * @param name The unique identifier used when adding the spritesheet.
         * @return const Spritesheet* Pointer to the spritesheet, or nullptr if not found.
         */
        const Spritesheet* getSpritesheet(const std::string& name) const;

        // ==============================
        // Textures
        // ==============================

        /**
         * @brief Loads an sf::Texture from file and stores it under a unique name.
         * @param name The unique identifier for the texture.
         * @param filepath Path to the image file.
         *
         * Useful for standalone images not part of a spritesheet (backgrounds, UI).
         */
        void addTexture(const std::string& name, const std::string& filepath);

        /**
         * @brief Retrieves a pointer to a stored sf::Texture.
         * @param name The unique identifier used when adding the texture.
         * @return const sf::Texture* Pointer to the texture, or nullptr if not found.
         */
        const sf::Texture* getTexture(const std::string& name) const;

        // ==============================
        // Fonts
        // ==============================

        /**
         * @brief Loads an sf::Font from file and stores it under a unique name.
         * @param name The unique identifier for the font.
         * @param filepath Path to the font file (e.g., .ttf).
         */
        void addFont(const std::string& name, const std::string& filepath);

        /**
         * @brief Retrieves a pointer to a stored sf::Font.
         * @param name The unique identifier used when adding the font.
         * @return const sf::Font* Pointer to the font, or nullptr if not found.
         */
        const sf::Font* getFont(const std::string& name) const;
    };

}
