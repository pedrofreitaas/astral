#include "Item.h"
#include "Zoe.h"
#include "../core/Game.h"

class TV : protected Item
{
public:
    TV(Game *game, const Vector2 &position);
    void OnUpdate(float deltaTime) override;
    void ManageAnimations();

    void OnPick() override;

    Timer *mTurnOnTimer;
};