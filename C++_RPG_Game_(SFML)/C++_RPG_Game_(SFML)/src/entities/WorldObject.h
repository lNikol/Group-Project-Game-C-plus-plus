#pragma once
#include "AnimatedEntity.h"
#include "worldmap/Player.h"

namespace RPG {

	class WorldObject : public AnimatedEntity {
	public:
		virtual void onInteract(Player& player) = 0;
		virtual bool isSolid() const;
	};

}

