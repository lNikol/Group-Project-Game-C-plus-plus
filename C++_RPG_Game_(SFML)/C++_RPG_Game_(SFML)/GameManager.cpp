#include "GameManager.h"

namespace RPG {

	GameManager::GameManager() 
		: window(sf::VideoMode({ Window::WIDTH, Window::HEIGHT }), Window::TITLE),
		running(false)
	{
		window.setFramerateLimit(Window::BASE_FPS);
		spritesheetManager.addSpritesheet("test", "test.jpg");
		rect.setTexture(spritesheetManager.getSpritesheet("test"));
		rect.setPosition({0,0});
		rect.setSize({ 64, 64 });
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

		// Eventy - przeklejony z bazowego kodu SFML narazie
		while (const std::optional event = window.pollEvent()) {
			if (event->is<sf::Event::Closed>()) {
				window.close();
			}
			else if (const auto* keyPressed = event->getIf<sf::Event::KeyPressed>()) {
				if (keyPressed->scancode == sf::Keyboard::Scancode::Escape) {
					window.close();
				}
			}
		}

	}

	void GameManager::update(float dt) {

		std::cout << (1.f / dt) << " \n";

	}

	void GameManager::draw() {
		window.clear(sf::Color(0x4B0082FF));
		window.draw(rect);
		window.display();
	}

}