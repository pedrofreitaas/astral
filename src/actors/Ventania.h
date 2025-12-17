#pragma once
#include <SDL.h>
#include "Actor.h"
#include "Projectile.h"
#include "Collider.h"
#include "Ventania.h"
#include "../components/collider/AABBColliderComponent.h"
#include "Tile.h"
#include "ZoeFireball.h"
#include "../core/Game.h"
#include "../components/draw/DrawAnimatedComponent.h"
#include "../ui/DialogueSystem.h"
#include "../components/TimerComponent.h"
#include "./Collider.h"
#include "./traps/Spikes.h"
#include "./traps/Spear.h"
#include "./traps/Shuriken.h"
#include "../libs/Math.h"
#include "enemies/Sith.h"

class Ventania : public Actor
{
public:
    Ventania(Game* game, Vector2 playerCenter, Vector2 playerMoveDir, float forwardSpeed = .0f);

private:    
    class DrawAnimatedComponent *mDrawAnimatedComponent;
    void AnimationEndCallback(std::string animationName);
};
