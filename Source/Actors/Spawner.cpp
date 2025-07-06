//
// Created by Lucas N. Ferreira on 30/09/23.
//

#include "Spawner.h"
#include "../Game.h"
#include "Punk.h"
#include "Goomba.h"
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
                Vector2(200.0f, 200.0f),  // Grupo 1
                Vector2(800.0f, 300.0f),  // Grupo 2
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
                    //SDL_Log("Enemy spawned at: %.1f, %.1f", spawnPos.x, spawnPos.y);
                }
            }
        }
        else {
            int enemyNumber2 = Math::RandRangeInt(2, 4);
            for (int i = 0; i < enemyNumber2; i++) {
                float randX = Math::RandRange(50.0f, 100.0f);
                float randY = Math::RandRange(-50.0f, 500.0f);
                auto enemyFirstBlock = new Enemy(mGame, const_cast<Punk*>(punk), 1);
                Vector2 pos = GetPosition() + Vector2(randX, randY);
                //SDL_Log("Enemy spawned at: %.1f, %.1f", pos.x, pos.y);
                float randX2 = Math::RandRange(450.0f, 700.0f);
                float randY2 = Math::RandRange(-50.0f, 500.0f);
                enemyFirstBlock->SetPosition(pos);
                auto enemySecondBlock = new Enemy(mGame, const_cast<Punk*>(punk), 1);
                Vector2 pos2 = GetPosition() + Vector2(randX2, randY2);
                //SDL_Log("Enemy spawned at: %.1f, %.1f", pos2.x, pos2.y);
                float randX3 = Math::RandRange(-100.0f, 100.0f);
                float randY3 = Math::RandRange(600.0f, 1000.0f);
                enemySecondBlock->SetPosition(pos2);
                auto enemyThirdBlock = new Enemy(mGame, const_cast<Punk*>(punk), 1);
                Vector2 pos3 = GetPosition() + Vector2(randX3, randY3);
                //SDL_Log("Enemy spawned at: %.1f, %.1f", pos3.x, pos3.y);
                enemyThirdBlock->SetPosition(pos3);
            }

        }

    SetState(ActorState::Destroy);
}
