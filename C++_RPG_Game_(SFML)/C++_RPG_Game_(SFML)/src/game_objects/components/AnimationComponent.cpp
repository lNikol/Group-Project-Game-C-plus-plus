#include "AnimationComponent.h"
#include "game_objects/GameObject.h"

namespace RPG {

	void AnimationComponent::init() {
		renderComponent = owner->getComponent<RenderComponent>();

		assert(renderComponent != nullptr
			&& "AnimationComponent added to a GameObject missing a RenderComponent!");
	}

	void AnimationComponent::setAnimations(std::map<std::string, Animation> animations) {
		this->animations = std::move(animations);
	}

	void AnimationComponent::loadAnimation(const std::string& name, Animation animation) {
		animations[name] = std::move(animation);
	}

	void AnimationComponent::play(const std::string& name) {
		if (currentAnimation != &animations[name]) {
			currentAnimation = &animations[name];
			currentAnimation->reset();

			sf::Sprite& sprite = renderComponent->getSprite();
			sf::IntRect frame = currentAnimation->getCurrentFrame();
			sprite.setTextureRect(frame);
			sprite.setOrigin({ frame.size.x / 2.0f, frame.size.y / 2.0f });
		}
	}

	void AnimationComponent::update(float dt) {
		if (currentAnimation) {
			currentAnimation->update(dt);
			renderComponent->getSprite().setTextureRect(currentAnimation->getCurrentFrame());
		}
	}
}
