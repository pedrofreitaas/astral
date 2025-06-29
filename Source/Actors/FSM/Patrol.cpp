//
// Created by eduar on 24/06/2025.
//


#include "Patrol.h"
#include "Chase.h"
#include "../Enemy.h"
#include "../Punk.h" // Precisamos saber a posição do player
#include "../../Components/DrawComponents/DrawAnimatedComponent.h"

// Vamos precisar do estado de perseguição para fazer a transição
//#include "EstadoPerseguindo.h"

// Construtor para inicializar as variáveis
Patrol::Patrol(float patrolDistance)
    : mPatrolDistance(patrolDistance)
    , mCurrentDirection(1) // Começa movendo para a direita
{
}

void Patrol::Enter(Enemy* enemy) {
    // 1. Guarda a posição inicial para calcular os limites da patrulha
    mStartPoint = enemy->GetPosition();
    //SDL_Log("Patrol::Enter mStartPoint %f" ,mStartPoint.x);
    mCurrentDirection = 1; // Garante que sempre começa indo para a direita

    // 2. Define a animação de andar/correr (use a que tiver, "run" ou "perseguindo" serve)
    enemy->GetDrawComponent()->SetAnimation("run");
    enemy->SetRotation(0.0f); // Garante que o sprite está virado para a direita
}

void Patrol::Update(Enemy* enemy, float deltaTime) {
    // --- Lógica de Transição ---
    // Verifica a distância para o jogador
    Vector2 posEnemy = enemy->GetPosition();
    //SDL_Log("Patrol::posEnemy %f", posEnemy.x);
    Vector2 posPunk = enemy->GetPunk()->GetPosition();
    Vector2 diff = posPunk - posEnemy;
    float distancia = diff.Length();

    // Se o jogador estiver dentro de um raio de 300 pixels, muda para o estado de perseguição
    if (distancia < 250.0f) {
        enemy->GetDrawComponent()->SetAnimation("jump");
        enemy->ChangeState(std::make_unique<Chase>());
        return;
    }

    // --- Lógica de Ação ---
    // 1. Define os limites da patrulha
    float leftBoundary = mStartPoint.x - mPatrolDistance;
    float rightBoundary = mStartPoint.x + mPatrolDistance;

    Vector2 currentPos = enemy->GetPosition();

    // 2. Verifica se atingiu os limites e inverte a direção
    if (mCurrentDirection == 1 && currentPos.x >= rightBoundary) {
        mCurrentDirection = -1; // Vira para a esquerda
        enemy->SetRotation(Math::Pi); // Rotação de 180 graus para virar o sprite
    }
    else if (mCurrentDirection == -1 && currentPos.x <= leftBoundary) {
        mCurrentDirection = 1; // Vira para a direita
        enemy->SetRotation(0.0f);
    }

    // 3. Aplica a força para mover o inimigo
    RigidBodyComponent* rb = enemy->GetRigidBody();
    if (rb) {
        float forwardSpeed = enemy->GetVelocidade(); // Pega a velocidade definida no Inimigo
        rb->ApplyForce(Vector2(mCurrentDirection * forwardSpeed, 0.0f));
    }

}

void Patrol::Exit(Enemy* enemy) {
    // Nada a limpar neste estado
    //SDL_Log("Patrol::Exit");
    RigidBodyComponent* rb = enemy->GetRigidBody();
    if (rb) {
        // Zera apenas a velocidade horizontal, mantendo a vertical (caso ele esteja caindo)
        rb->SetVelocity(Vector2(0.0f, rb->GetVelocity().y));
    }
}