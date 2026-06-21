#pragma once
#include "game_objects/components/Component.h"

namespace RPG {

    class DialogComponent : public Component {
    public:
        int dialogId = -1;

        DialogComponent(int id) : dialogId(id) {}
    };

}
