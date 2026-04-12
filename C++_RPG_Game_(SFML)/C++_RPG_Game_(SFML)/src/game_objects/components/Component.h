#pragma once
#include <SFML/Graphics.hpp>
#include "core/Constants.h"

namespace RPG {

	class GameObject;

	class Component {
	protected:
		GameObject* owner = nullptr;

	public:
		virtual ~Component() = default;
		virtual void init();
		virtual void update(float dt);
		virtual void draw(sf::RenderWindow& window);

		void setOwner(GameObject* gameObject);
		GameObject* getOwner() const;
	};

}
