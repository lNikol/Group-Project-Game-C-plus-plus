#include "Component.h"

namespace RPG {

	void Component::init() {
	}

	void Component::update(float dt) {
	}

	void Component::draw(sf::RenderWindow& window) {
	}
	
	GameObject* Component::getOwner() const {
		return this->owner;
	}

	void Component::setOwner(GameObject* gameObject) {
		this->owner = gameObject;
	}

}