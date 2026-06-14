#pragma once
#include "game_objects/components/ColliderComponent.h"
#include "game_objects/components/RenderComponent.h"
#include "game_objects/GameObject.h"
#include <vector>
#include <memory>

namespace RPG {

    class DebugSystem {
        bool enabled = false;
    public:
        void toggle(std::vector<std::unique_ptr<GameObject>>& entities) {
            enabled = !enabled;
            for (auto& e : entities) {
                if (auto* c = e->getComponent<ColliderComponent>()) c->setDebug(enabled);
                if (auto* r = e->getComponent<RenderComponent>())   r->setDebug(enabled);
            }
        }
        bool isEnabled() const { return enabled; }
    };

}
