#include "StructureComponent.h"

namespace RPG {


	StructureComponent::StructureComponent(StructureDefinition* definition)
		: definition(definition)
	{
	}

	const StructureDefinition& StructureComponent::getDefinition() const
	{
		return *definition;
	}

}