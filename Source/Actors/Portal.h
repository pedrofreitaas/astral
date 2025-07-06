//
// Created by Pedro on 29/06/2025.
//

#ifndef PORTAL_H
#define PORTAL_H
#include "./Actor.h"
#include "../Components/DrawComponents/DrawAnimatedComponent.h"
#include "../Components/ColliderComponents/AABBColliderComponent.h"

class Portal: public Actor {
    public:
        Portal(Game* game, int type = 0);
        ~Portal();

    private:
        AABBColliderComponent* mColliderComponent;
        DrawAnimatedComponent* mDrawComponent;
};

#endif