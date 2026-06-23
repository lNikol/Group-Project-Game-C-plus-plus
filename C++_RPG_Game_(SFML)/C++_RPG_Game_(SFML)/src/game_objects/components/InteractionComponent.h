#pragma once
#include <SFML/Graphics.hpp>
#include "Component.h"
#include "animation/Animation.h"
#include <map>
#include <string>
#include <optional>

namespace RPG {

	class InteractionComponent : public Component {
		float interactionRadius;
		float m_distanceToPlayer = 9999.f;
		float m_animationTime = 0.f;
		
		std::map<std::string, Animation> m_animations;
		Animation* m_currentAnim = nullptr;
		std::string m_currentState = "";
		std::optional<sf::Sprite> m_sprite;

		void setAnimationState(const std::string& state);

	public:
		InteractionComponent(float interactionRadius);

		float getInteractionRadius() const;
		void setDistanceToPlayer(float dist) { m_distanceToPlayer = dist; }

		void init() override;
		void update(float dt) override;
		void draw(sf::RenderWindow& window) override;
	};

}
