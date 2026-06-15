#include "Item.h"
#include "Zoe.h"
#include "../core/Game.h"

class Dog : protected Item
{
public:
    Dog(Game *game, const Vector2 &position);

    void OnPick() override;
    void OnUpdate(float deltaTime) override;
    
    void ManageAnimations();
    void AnimationEndCallback(const std::string &animationName);

    void PickCallback();
    void Bark();
    void Lick();
};