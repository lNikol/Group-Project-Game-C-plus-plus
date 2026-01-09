#include "core/GameManager.h"

namespace RPG {

    GameManager::GameManager()
        : window(sf::VideoMode({ Window::WIDTH, Window::HEIGHT }), Window::TITLE),
        running(false)
    {
        window.setFramerateLimit(Window::BASE_FPS);

        // Assets and structures init
        assetManager.init(spritesheetManager);

        spritesheetManager.addSpritesheet(GameConfig::TEXTURES_PATH + "hero.png"); 

        worldMap = std::make_unique<WorldMap>(50, 50, assetManager);

        // Create a player in the middle of the map
        player = std::make_unique<Player>(200.0f, 200.0f);

        worldMap->generateObstacles(0.07f, player->getPosition(), 3);

        // Test building placement
        worldMap->placeStructure(15, 15, StructureType::Camp);

        // Set camera
        camera.setSize(sf::Vector2f(Window::WIDTH, Window::HEIGHT));
    }

    void GameManager::run() {
        running = true;
        sf::Clock clock;

        while (window.isOpen() && running) {
            float dt = clock.restart().asSeconds();
            handleEvents();
            update(dt);
            draw();
        }
    }

    void GameManager::handleEvents() {
        while (const std::optional event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>()) window.close();

            if (const auto* keyPressed = event->getIf<sf::Event::KeyPressed>()) {
                if (keyPressed->scancode == sf::Keyboard::Scancode::Escape) running = false;
            }
        }
    }

    void GameManager::update(float dt) {
        if (player && worldMap) {
            player->update(dt, *worldMap, window);

            camera.setCenter(player->getPosition());
        }
    }

    void GameManager::draw() {
        window.clear(sf::Color(0x4B0082FF)); 

        window.setView(camera);

        if (worldMap && player) {
            worldRenderer.draw(window, *worldMap, assetManager, spritesheetManager, 0.8f); // set view in percents depends on scene (enum) each scene has own % of view

            player->draw(window, spritesheetManager);
        }

        window.display();
    }
}