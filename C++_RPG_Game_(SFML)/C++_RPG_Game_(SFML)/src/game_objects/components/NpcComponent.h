#pragma once
#include "Component.h"
#include "worldmap/enums.h"

namespace RPG {

	class NpcComponent : public Component {
		FactionID targetFaction;
		bool factionLeader;

	public:
		NpcComponent(FactionID targetFaction);

		FactionID getTargetFaction() const;

		void setFactionLeader(bool factionLeader);
		bool isFactionLeader() const;
	};

}

