//
// Created by Lucas N. Ferreira on 30/09/23.
//

#include "Spawner.h"
#include "../Game.h"
#include "Punk.h"
#include "Goomba.h"
#include "Enemy.h"

Spawner::Spawner(Game* game, float spawnDistance)
        :Actor(game)
        ,mSpawnDistance(spawnDistance)
{

}

void Spawner::OnUpdate(float deltaTime)
{
    auto *punk = mGame->GetPunk();

    if (punk == nullptr) return;

    float deltaX = Math::Abs(punk->GetPosition().x - GetPosition().x);

    if (deltaX > mSpawnDistance) return;
    int enemyNumber = Math::RandRangeInt(2, 8);
    //SDL_Log("\nNumber of enemies: %d\n",enemyNumber);

    for (int i = 0; i < enemyNumber; i++)
    {
        // Gere posições aleatórias
        float randX = Math::RandRange(50.0f, 500.0f);
        float randY = Math::RandRange(-50.0f, 300.0f);
        auto enemy = new Enemy(mGame, const_cast<Punk*>(punk));
        Vector2 pos = GetPosition() + Vector2(randX, randY);
        //SDL_Log("Enemy spawned at: %.1f, %.1f", pos.x, pos.y);
        enemy->SetPosition(pos);
    }

    SetState(ActorState::Destroy);
}
