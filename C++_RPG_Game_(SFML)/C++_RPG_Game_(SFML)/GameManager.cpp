#include "GameManager.h"

namespace RPG {

	GameManager::GameManager() 
		: window(sf::VideoMode({ 800, 600 }), "My Game"),
		running(false)
	{

	}

	void GameManager::run() {
		running = true;

		while (window.isOpen() && running) {
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
			//Render
			window.clear(sf::Color(0x4B0082FF));

			//Drawing
			window.display();

		}
	}

}