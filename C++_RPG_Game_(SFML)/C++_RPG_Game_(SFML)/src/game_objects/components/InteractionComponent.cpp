#include "InteractionComponent.h"

namespace RPG {

	InteractionComponent::InteractionComponent(float interactionRadius)
		: interactionRadius(interactionRadius)
	{
	}

	float InteractionComponent::getInteractionRadius() const {
		return interactionRadius;
	}

}
