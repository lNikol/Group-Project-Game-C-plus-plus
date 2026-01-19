#pragma once
#include <SFML/Graphics.hpp>
#include <iostream>
#include "Constants.h"
#include "AssetManager.h"
#include "GUI/windows/BattleHUD.h"
#include "drawable/IsoGrid.h"
#include "placeholder/TestActor.cpp"

namespace RPG {

	class GameManager {

		bool running;
		sf::RenderWindow window;
		AssetManager assetManager;
		sf::RectangleShape rect;

		void update(float dt);
		void draw();
		void handleEvents();

	public:
		GameManager();

		void run();
	private:
		// BattleHUD
		const sf::Font* m_globalFont;
		const Spritesheet* m_iconSet;
		std::unique_ptr<TestActor> m_player;
		std::unique_ptr<BattleHUD> m_hud;
	};



}

