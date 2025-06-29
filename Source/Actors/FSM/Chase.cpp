//
// Created by eduar on 25/06/2025.
//

#include "Chase.h"
#include "Patrol.h" // Para a transição de volta para a patrulha
#include "Attack.h"
#include "../Enemy.h"
#include "../Punk.h"
#include "../../Components/RigidBodyComponent.h"
#include "../../Components/DrawComponents/DrawAnimatedComponent.h"
#include <SDL_log.h>

// A implementação do construtor que pode estar faltando
Chase::Chase() {
    //SDL_Log("Chase constructor called");
}

void Chase::Enter(Enemy* enemy) {
    //SDL_Log("PursueState::Enter");
    // Ao começar a perseguir, garante que a animação de corrida está ativa
    enemy->GetDrawComponent()->SetAnimation("run");
}

void Chase::Update(Enemy* enemy, float deltaTime) {
    enemy->MoveTowardsPlayer();

    Vector2 enemyPos = enemy->GetPosition();
    Vector2 punkPos = enemy->GetPunk()->GetPosition();

    // Calcula o vetor que aponta do inimigo para o punk
    Vector2 direction = punkPos - enemyPos;
    float distance = direction.Length();

    // Define os alcances para tomar decisões.
    const float attackRange = 120.0f;    // Distância para começar a atacar
    const float loseSightRange = 400.0f; // Distância para desistir e voltar a patrulhar

    // Se o Punk se afastar demais, desiste e volta a patrulhar
    if (distance > loseSightRange) {
        enemy->ChangeState(std::make_unique<Patrol>());
        return;
    }

    // Se o Punk estiver perto o suficiente, muda para o estado de ataque
    if (distance < attackRange) {
        //SDL_Log("In range to attack!");
        enemy->ChangeState(std::make_unique<Attack>());
        return;
    }
}

void Chase::Exit(Enemy* enemy) {
    //SDL_Log("PursueState::Exit");
    RigidBodyComponent* rb = enemy->GetRigidBody();
    if (rb) {
        rb->SetVelocity(Vector2::Zero);
    }
}