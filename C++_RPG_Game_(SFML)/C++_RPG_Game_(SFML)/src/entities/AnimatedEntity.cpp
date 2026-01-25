#include "AnimatedEntity.h"

namespace RPG {

	static sf::Texture dummyTexture;

	AnimatedEntity::AnimatedEntity()
		: currentAnimation(nullptr), sprite(dummyTexture)
	{
	}

	void AnimatedEntity::loadAnimation(const std::string& name, Animation animation) {
		animations[name] = std::move(animation);
	}

	void AnimatedEntity::play(const std::string& name) {
		if (currentAnimation != &animations[name]) {
			currentAnimation = &animations[name];
			currentAnimation->reset();
			sprite.setTexture(*currentAnimation->getSpritesheet());
			sf::IntRect frame = currentAnimation->getCurrentFrame();
			sprite.setOrigin({ frame.size.x / 2.0f, frame.size.y / 2.0f });
		}
	}

	void AnimatedEntity::update(float dt) {
		if (currentAnimation) {
			currentAnimation->update(dt);
			sprite.setTextureRect(currentAnimation->getCurrentFrame());
		}
	}

	void AnimatedEntity::draw(sf::RenderWindow& window) {
		window.draw(sprite);
	}

	sf::Vector2f AnimatedEntity::getPosition() const {
		return sprite.getPosition();
	}

	sf::FloatRect AnimatedEntity::getGlobalBounds() const {
		return sprite.getGlobalBounds();
	}

	sf::FloatRect AnimatedEntity::getLocalBounds() const {
		return sprite.getLocalBounds();
	}

	void AnimatedEntity::setPosition(const sf::Vector2f& position) {
		sprite.setPosition(position);
	}

	void AnimatedEntity::setPosition(float x, float y) {
		setPosition({ x, y });
	}

	void AnimatedEntity::move(const sf::Vector2f& offset) {
		sprite.move(offset);
	}

	void AnimatedEntity::move(float offsetX, float offsetY) {
		move({ offsetX, offsetY });
	}

}