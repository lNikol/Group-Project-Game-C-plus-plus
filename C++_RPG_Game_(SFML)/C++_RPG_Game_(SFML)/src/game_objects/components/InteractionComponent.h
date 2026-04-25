#pragma once
#include "Component.h"

namespace RPG {

	class InteractionComponent : public Component {
		float interactionRadius;

	public:
		InteractionComponent(float interactionRadius);

		float getInteractionRadius() const;
	};

}

