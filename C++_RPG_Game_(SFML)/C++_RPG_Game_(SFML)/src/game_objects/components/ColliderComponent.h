#pragma once
#include "Component.h"
#include <iostream>

namespace RPG {

	class ColliderComponent : public Component {
		sf::FloatRect localHitbox;
		bool showDebug = false;
	public:
		ColliderComponent(float width, float height, float offsetX = 0.f, float offsetY = 0.f);
		ColliderComponent(sf::Vector2f size, sf::Vector2f offset = {0,0});

		void draw(sf::RenderWindow& window) override;

		sf::FloatRect getGlobalHitbox() const;
		sf::FloatRect getLocalHitbox() const;
		void setDebug(bool show);
		void toggleDebug();
	};

}

