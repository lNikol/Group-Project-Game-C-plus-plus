#pragma once
#include <SFML/Graphics.hpp>
#include <map>
#include "components/Component.h"

namespace RPG {

	class GameObject {
		std::string prefabId;
		sf::Vector2f position;
		std::vector<std::unique_ptr<Component>> components;
	public:
		GameObject() = default;

		void update(float dt);
		void draw(sf::RenderWindow& window);

		// Getters & Setters

		void setPosition(sf::Vector2f position);
		void setPosition(float x, float y);
		sf::Vector2f getPosition() const;
		const std::string& getPrefabId() const;
		void setPrefabId(const std::string& prefabId);


		// Component manipulation

		template<class T, class... Args>
		T* addComponent(Args&&... args) {
			auto newComponent = std::make_unique<T>(std::forward<Args>(args)...);
			T* ptr = newComponent.get();
			ptr->setOwner(this);
			components.push_back(std::move(newComponent));
			ptr->init();
			return ptr;
		}

		template<class T>
		T* addComponent(std::unique_ptr<T> component) {
			T* ptr = component.get();
			ptr->setOwner(this);
			components.push_back(std::move(component));
			ptr->init();
			return ptr;
		}

		template<class T>
		T* getComponent() {
			for (auto& c : this->components) {
				T* target = dynamic_cast<T*>(c.get());
				if (target) { return target; }
			}
			return nullptr;
		}

		template<class T>
		std::vector<T*> getComponents() {
			std::vector<T*> res;
			for (auto& c : this->components) {
				if (auto ptr = dynamic_cast<T*>(c.get())) {
					res.push_back(ptr);
				}
			}
			return res;
		}

		template<class T>
		bool hasComponent() {
			return getComponent<T>() != nullptr;
		};
	};

}

