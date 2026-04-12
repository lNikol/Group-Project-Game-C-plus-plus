#pragma once
#include "Component.h"
#include <iostream>

namespace RPG {

	class ColliderComponent : public Component {
		sf::FloatRect localHitbox;
		bool showDebug = false;
	public:
		ColliderComponent(float width, float height, float offsetX = 0.f, float offsetY = 0.f);

		void draw(sf::RenderWindow& window) override;

		sf::FloatRect getGlobalHitbox() const;
		sf::FloatRect getLocalHitbox() const;
		void setDebug(bool show);
	};

}

