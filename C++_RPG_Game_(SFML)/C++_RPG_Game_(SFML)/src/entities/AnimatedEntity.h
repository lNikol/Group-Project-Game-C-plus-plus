#pragma once
#include <SFML/Graphics.hpp>
#include <map>
#include <optional>
#include "animation/Animation.h"

namespace RPG {

	class AnimatedEntity {
	protected:
		std::map<std::string, Animation> animations;
		Animation* currentAnimation;
		sf::Sprite sprite;
		
	public:
		AnimatedEntity();
		virtual ~AnimatedEntity() = default;

		void loadAnimation(const std::string& name, Animation animation);
		void play(const std::string& name);
		virtual void update(float dt);
		void draw(sf::RenderWindow& window);

		sf::Vector2f getPosition() const;
		sf::FloatRect getGlobalBounds() const;
		sf::FloatRect getLocalBounds() const;

		void setPosition(const sf::Vector2f& position);
		void setPosition(float x, float y);

		void move(float offsetX, float offsetY);
		void move(const sf::Vector2f& offset);
	};

}

