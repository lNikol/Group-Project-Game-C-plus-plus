#include "GameObject.h"

namespace RPG {

	GameObject::GameObject(const std::string& prefabId)
		: prefabId(prefabId), zIndex(0), position({ 0,0 })
	{}

	void GameObject::update(float dt) {
		for (const auto& e : components) {
			e->update(dt);
		}
	}

	void GameObject::draw(sf::RenderWindow& window) {
		for (const auto& e : components) {
			e->draw(window);
		}
	}


	// Getters & Setters

	void GameObject::setPosition(sf::Vector2f position) {
		this->position = position;
	}

	void GameObject::setPosition(float x, float y) {
		this->position.x = x;
		this->position.y = y;
	}

	sf::Vector2f GameObject::getPosition() const {
		return position;
	}

	const std::string& GameObject::getPrefabId() const {
		return prefabId;
	}

	void GameObject::setPrefabId(const std::string& prefabId) {
		this->prefabId = prefabId;
	}

	uint8_t GameObject::getZIndex() const {
		return zIndex;
	}

	void GameObject::setZIndex(uint8_t zIndex) {
		this->zIndex = zIndex;
	}

}