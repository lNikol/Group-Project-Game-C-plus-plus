#pragma once
#include "worldmap/enums.h"

namespace RPG {

    /**
     * @class ISceneController
     * @brief Interface to allow scenes to request global state changes
     * without knowing the full implementation of GameManager.
     */
    class ISceneController {
    public:
        virtual ~ISceneController() = default;

        // Only the necessary methods for the Scene to call
        virtual void changeScene(FactionID target) = 0;
        virtual bool hasWindowFocus() const = 0;
    };
}