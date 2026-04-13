#pragma once
#include "Component.h"
#include "worldmap/StructureDefinition.h"

namespace RPG {

	class StructureComponent : public Component
	{
		StructureDefinition* definition;

	public:
		StructureComponent(StructureDefinition* definition);

		const StructureDefinition& getDefinition() const;
	};

}

