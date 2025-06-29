//
// Created by eduar on 27/06/2025.
//

#include "Attack.h"
#include "Chase.h"

#include "../Enemy.h"
#include "../Punk.h"
#include "../../Components/DrawComponents/DrawAnimatedComponent.h"
#include <SDL_log.h>

Attack::Attack(float mAttackAnimTime)
: mAttackAnimTime(mAttackAnimTime), mTimer(0.0f)
{
}

void Attack::Enter(Enemy* enemy) {
    //SDL_Log("ENEMY: Entering ATTACK state (Ranged).");

    //enemy->GetDrawComponent()->SetAnimation("attack"); // Inicia a animação de ataque

    //Fica travado quando está mirando
    mTimer = mAttackAnimTime; // Usando a variável do construtor do AttackState
}

void Attack::Update(Enemy* enemy, float deltaTime) {
    // 1. Continua se movendo em direção ao jogador, mesmo no estado de ataque!
    enemy->MoveTowardsPlayer();

    // 2. Tenta atirar (a função do inimigo vai checar o cooldown da arma)
    enemy->ShootAtPlayer(enemy->GetPunk()->GetPosition(), enemy->GetPunk()->GetComponent<AABBColliderComponent>());

    // --- LÓGICA DE TRANSIÇÃO ---

    // Decrementa o cronômetro do estado
    mTimer -= deltaTime;
    if (mTimer <= 0.0f) {
        // Quando o tempo na "fase de ataque" acaba, volta para a perseguição
        // para reavaliar se continua atacando ou se o jogador fugiu.
        enemy->ChangeState(std::make_unique<Chase>());
    }
}

void Attack::Exit(Enemy* enemy) {
    //SDL_Log("ENEMY: Exiting ATTACK state.");
}