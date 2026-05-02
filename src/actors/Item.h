#ifndef ASTRAL_ITEM_H
#define ASTRAL_ITEM_H

#include <functional>
#include <utility>
#include <string>
#include "Actor.h"
#include "Button.h"
#include "../core/Game.h"
#include "../components/draw/DrawAnimatedComponent.h"
#include "../components/collider/AABBColliderComponent.h"
#include "../components/RigidBodyComponent.h"

class Item : public Actor
{
using PickHandler = std::function<void(Item &)>;

public:
    //default constructor
    Item() = default;

    static Item* CreateNevascaItem(Game *game, const Vector2& position);
    static Item* CreateVentaniaItem(Game *game, const Vector2& position);
    static Item* CreateFireballItem(Game *game, const Vector2& position);
    static Item* CreateDodgeItem(Game *game, const Vector2& position);
    
private:
    Item(
        Game *game, 
        const Vector2& position, 
        std::string texturePath,
        std::string dataPath,
        int bbWidth, int bbHeight,
        PickHandler onPickCallback = nullptr,
        Button button = Button::None,
        int animationStartIdx=0, int animationEndIdx=0,
        float animFPS=10.f
    );
    
    bool mIsPicked, mIsPickable;
    PickHandler mOnPickCallback;

    DrawAnimatedComponent *mDrawComponent, *mButtonDrawComponent;
    AABBColliderComponent *mColliderComponent;
    RigidBodyComponent *mRigidBodyComponent;

    void OnUpdate(float deltaTime) override;
    void OnProcessInput(const Uint8* keyState, const std::vector<SDL_Event>& events) override;
    void Kill() override;

    Button mButton;
};

#endif