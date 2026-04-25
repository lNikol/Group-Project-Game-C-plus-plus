#include "NpcComponent.h"

namespace RPG {

	NpcComponent::NpcComponent(FactionID targetFaction) 
		: targetFaction(targetFaction)
	{
		factionLeader = false;
	}

	FactionID NpcComponent::getTargetFaction() const {
		return targetFaction;
	}

	void NpcComponent::setFactionLeader(bool factionLeader) {
		this->factionLeader = factionLeader;
	}

	bool NpcComponent::isFactionLeader() const {
		return factionLeader;
	}

}
