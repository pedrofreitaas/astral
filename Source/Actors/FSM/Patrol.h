//
// Created by eduar on 24/06/2025.
//

#ifndef PATROL_H
#define PATROL_H

#include "State.h"
#include "../../Math.h"

class Patrol : public State {

public:
    Patrol(float patrolDistance = 100.0f);

    void Enter(Enemy *enemy) override;
    void Exit(Enemy *enemy) override;
    void Update(Enemy *enemy, float dt) override;
private:
    Vector2 mStartPoint;      // Posição onde a patrulha começou
    float mPatrolDistance;    // Distância máxima para patrulhar
    int mCurrentDirection;    // Direção do movimento: 1 para direita, -1 para esquerda
};

#endif //PATROL_H
