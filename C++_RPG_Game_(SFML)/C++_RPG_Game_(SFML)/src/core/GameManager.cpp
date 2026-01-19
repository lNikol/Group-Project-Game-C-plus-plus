#include "GameManager.h"

namespace RPG {

	GameManager::GameManager() 
		: window(sf::VideoMode({ Window::WIDTH, Window::HEIGHT }), Window::TITLE),
		running(false)
	{
		window.setFramerateLimit(Window::BASE_FPS);

		assetManager.addSpritesheet("AbilityIcons", "assets/textures/placeholders/IconSet.png");
		assetManager.addFont("PixelFont", "assets/fonts/m5x7.ttf");
		m_globalFont = assetManager.getFont("PixelFont");
		m_iconSet = assetManager.getSpritesheet("AbilityIcons");
		if (m_globalFont == nullptr) {
			std::cerr << "Failed to load font!\n";
		}
		if (m_iconSet == nullptr) {
			std::cerr << "Failed to load IconSet!\n";
		}


		m_player = std::make_unique<TestActor>();
		m_hud = std::make_unique<BattleHUD>(*m_iconSet, *m_globalFont);
		m_hud->setCombatActor(m_player.get());
		m_hud->onResize(window.getSize());

		assetManager.addSpritesheet("test", "assets/textures/placeholders/test.jpg");
		rect.setTexture(assetManager.getSpritesheet("test"));
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

		// Eventy
		while (const std::optional event = window.pollEvent()) {
			m_hud->handleEvent(window, *event);
			if (event->is<sf::Event::Closed>()) {
				window.close();
			}
			else if (const auto* resized = event->getIf<sf::Event::Resized>()) {
				sf::Vector2f newSize = { (float)resized->size.x, (float)resized->size.y };
				sf::FloatRect visibleArea({ 0, 0 }, newSize);
				window.setView(sf::View(visibleArea));

				m_hud->onResize(resized->size);
			}
			else if (const auto* keyPressed = event->getIf<sf::Event::KeyPressed>()) {
				if (keyPressed->scancode == sf::Keyboard::Scancode::Escape) {
					window.close();
				}
			}
		}

	}

	void GameManager::update(float dt) {

		//std::cout << (1.f / dt) << " \n";
		if (m_hud) {
			m_hud->update(dt);
		}
	}

	void GameManager::draw() {
		window.clear(sf::Color(0x4B0082FF));
		window.draw(rect);
		IsoGrid BGGrid(12, 17, 70.f, sf::Color(100, 100, 150)); //background
		BGGrid.setPosition({ 0.f, 0.f });
		window.draw(BGGrid);
		if (m_hud) {
			window.draw(*m_hud);
		}
		window.display();
	}

}