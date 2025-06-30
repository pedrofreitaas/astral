//
// Created by eduar on 25/06/2025.
//

#ifndef ENEMY_H
#define ENEMY_H
#include <memory>
#include "Actor.h"
#include "Punk.h"
#include "../Game.h"
#include "FSM/State.h"

class Enemy: public Actor {
    public:
        Enemy(Game* game, Punk* player);
        ~Enemy();

        void OnUpdate(float deltaTime) override;
        void Kill() override;
        void ChangeState(std::unique_ptr<State> newState);

        void TakeDamage();
        void SetFacingLeft(bool isLeft);
        void Start();
        void OnHorizontalCollision(const float minOverlap, AABBColliderComponent* other) override;
        void OnVerticalCollision(const float minOverlap, AABBColliderComponent* other) override;

        void ShootAtPlayer(Vector2 targetPos, AABBColliderComponent* targetComponent); //Shooting related

        // Getters para os Estados
        Punk* GetPunk() { return mPunk; }
        RigidBodyComponent* GetRigidBody() { return mRigidBodyComponent; }
        DrawAnimatedComponent* GetDrawComponent() { return mDrawComponent; }
        float GetVelocidade() const { return mVelocidade; }
        void MoveTowardsPlayer();
        void DrawSpriteDebug(SDL_Renderer* renderer);
        void DrawColliderDebug(SDL_Renderer* renderer);

    private:
        // Ponteiros para os componentes, igual ao Punk
        RigidBodyComponent* mRigidBodyComponent;
        AABBColliderComponent* mColliderComponent;
        DrawAnimatedComponent* mDrawComponent;

        // O cérebro da FSM
        std::unique_ptr<State> mEstadoAtual;

        // Dados específicos do inimigo
        Punk* mPunk; // Precisa de uma referência ao player para saber quem seguir/atacar
        float mVelocidade;
        bool mIsDying;
        float mDeathTimer;

        int mHP = 3;
        bool mTakingDamage = false;
        float mDamageTimer = 0.0f;

        bool mIsShooting; //Shooting related
        float mFireCooldown; //Shooting related
};

#endif //ENEMY_H
