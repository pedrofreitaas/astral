//
// Created by eduar on 25/06/2025.
//

#ifndef CHASE_H
#define CHASE_H
#include "State.h"
#include "../../Math.h"

class Chase : public State {

public:
    Chase();

    void Enter(Enemy *enemy) override;
    void Exit(Enemy *enemy) override;
    void Update(Enemy *enemy, float dt) override;
};

#endif //CHASE_H