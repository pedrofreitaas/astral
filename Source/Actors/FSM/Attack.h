//
// Created by eduar on 27/06/2025.
//

#ifndef ATTACK_H
#define ATTACK_H


#pragma once
#include "State.h" // Inclui a definição da interface base 'Estado'

class Attack : public State {
public:
    // O construtor permite definir quanto tempo a animação de ataque dura
    Attack(float attackAnimTime = 0.5f);

    // Sobrescreve as funções da FSM
    void Enter(Enemy* enemy) override;
    void Update(Enemy* enemy, float deltaTime) override;
    void Exit(Enemy* enemy) override;

private:
    float mAttackAnimTime; // Duração da animação/pose de ataque
    float mTimer;          // Cronômetro para controlar essa duração
};
#endif //ATTACK_H
