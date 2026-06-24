#include "InteractionComponent.h"
#include "game_objects/GameObject.h"
#include "worldmap/AssetManager.h"
#include "animation/AnimationLoader.h"
#include <cmath>

namespace RPG {

	InteractionComponent::InteractionComponent(float interactionRadius)
		: interactionRadius(interactionRadius)
	{}

	void InteractionComponent::init() {
		const Spritesheet* indicatorSheet = AssetManager::getInstance().getSpritesheet("indicator");
		if (indicatorSheet) {
			m_animations = AnimationLoader::loadAnimations(
				GameConfig::ANIMATIONS_PATH + "indicator_animations.json",
				indicatorSheet
			);
			m_sprite.emplace(*indicatorSheet);
		}
	}

	float InteractionComponent::getInteractionRadius() const {
		return interactionRadius;
	}

	void InteractionComponent::setAnimationState(const std::string& state) {
		if (m_currentState != state) {
			m_currentState = state;
			auto it = m_animations.find(state);
			if (it != m_animations.end()) {
				m_currentAnim = &it->second;
				m_currentAnim->reset();
			} else {
				m_currentAnim = nullptr;
			}
		}
	}

	void InteractionComponent::update(float dt) {
		m_animationTime += dt;

		bool isClose = (m_distanceToPlayer <= interactionRadius);

		if (isClose) {
			if (m_currentState == "far_hover" || m_currentState == "") {
				setAnimationState("transition");
				if (m_currentAnim) {
					m_currentAnim->setReversed(false);
					m_currentAnim->reset();
				}
			} else if (m_currentState == "transition" && m_currentAnim && !m_currentAnim->isReversed()) {
				if (m_currentAnim->isFinished()) {
					setAnimationState("close_hover");
				}
			} else if (m_currentState == "transition" && m_currentAnim && m_currentAnim->isReversed()) {
				// Walking away but came close again
				m_currentAnim->setReversed(false);
				m_currentAnim->setFinished(false);
			} else if (m_currentState != "close_hover" && m_currentState != "transition") {
				setAnimationState("close_hover");
			}
		} else {
			if (m_currentState == "close_hover") {
				setAnimationState("transition");
				if (m_currentAnim) {
					m_currentAnim->setReversed(true);
					m_currentAnim->reset();
				}
			} else if (m_currentState == "transition" && m_currentAnim && m_currentAnim->isReversed()) {
				if (m_currentAnim->isFinished()) {
					setAnimationState("far_hover");
				}
			} else if (m_currentState == "transition" && m_currentAnim && !m_currentAnim->isReversed()) {
				// Coming close but walked away
				m_currentAnim->setReversed(true);
				m_currentAnim->setFinished(false);
			} else if (m_currentState != "far_hover" && m_currentState != "transition") {
				setAnimationState("far_hover");
			}
		}

		if (m_currentAnim) {
			m_currentAnim->update(dt);
		}
	}

	void InteractionComponent::draw(sf::RenderWindow& window) {
		if (!owner) return;

		if (!m_currentAnim || !m_sprite.has_value()) return;

		sf::Vector2f pos = owner->getPosition();
		
		// Bouncing offset
		float bounce = std::sin(m_animationTime * 3.f) * 3.f;
		pos.y -= (50.f + bounce); // Hover above NPC

		m_sprite->setTextureRect(m_currentAnim->getCurrentFrame());
		
		// Center the sprite origin
		sf::FloatRect bounds = m_sprite->getLocalBounds();
		m_sprite->setOrigin({bounds.position.x + bounds.size.x / 2.f, bounds.position.y + bounds.size.y / 2.f});
		m_sprite->setPosition(pos);

		window.draw(*m_sprite);
	}

}
