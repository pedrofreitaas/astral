//
// Created by Lucas N. Ferreira on 30/09/23.
//

#include "Spawner.h"
#include "../Game.h"
#include "Punk.h"
#include "Enemy.h"

Spawner::Spawner(Game* game, float spawnDistance, int type)
        :Actor(game)
        ,mSpawnDistance(spawnDistance), mType(type)
{

}

void Spawner::OnUpdate(float deltaTime)
{
    auto *punk = mGame->GetPunk();

    if (punk == nullptr) return;

    int enemyNumber = Math::RandRangeInt(4, 9);
    //SDL_Log("\nNumber of enemies: %d\n",enemyNumber);
        if (mType == 0 ) {

            std::vector<Vector2> groupCenters = {
                //Vector2(200.0f, 200.0f),  // Grupo 1
                Vector2(520.0f, 300.0f),  // Grupo 2
                Vector2(1200.0f, 1000.0f),  // Grupo 3
                Vector2(1400.0f, 400.0f)  // Grupo 4
            };

            for (const auto& center : groupCenters)
            {
                for (int i = 0; i < 3; i++) // 4 inimigos por grupo
                {
                    Vector2 offset;
                    switch (i) {
                        case 0: offset = Vector2(-30.0f, 0.0f); break;
                        case 1: offset = Vector2(0.0f, 0.0f); break;
                        case 2: offset = Vector2(30.0f, 0.0f); break;
                    }

                    auto enemy = new Enemy(mGame, const_cast<Punk*>(punk), 0);
                    Vector2 spawnPos = center + offset;
                    enemy->SetPosition(spawnPos);
                    enemy->Start();

                    //SDL_Log("Enemy spawned at: %.1f, %.1f", spawnPos.x, spawnPos.y);
                }
            }
        }
        else {
            int enemyNumber2 = Math::RandRangeInt(2, 4);

            // Centros dos 2 grupos
            std::vector<Vector2> groupCenters = {
                Vector2(430.0f, 700.0f),   // Grupo 1
                Vector2(430.0f, 330.0f)   // Grupo 2
            };

            for (const auto& center : groupCenters)
            {
                for (int i = 0; i < 3; i++)
                {
                    Vector2 offset;
                    switch (i)
                    {
                        case 0: offset = Vector2(-40.0f, -10.0f); break;
                        case 1: offset = Vector2(0.0f, 0.0f);     break;
                        case 2: offset = Vector2(40.0f, 20.0f);   break;
                    }

                    auto enemy = new Enemy(mGame, const_cast<Punk*>(punk), 1);
                    Vector2 spawnPos = center + offset;
                    enemy->SetPosition(spawnPos);
                    enemy->Start();

                    //SDL_Log("Enemy type 1 spawned at: %.1f, %.1f", spawnPos.x, spawnPos.y);
                }
            }

        }

    SetState(ActorState::Destroy);
}
