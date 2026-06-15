#include "Zathura.h"
#include "ZathuraRock.h"
#include "../../core/Game.h"
#include "../../components/draw/DrawAnimatedComponent.h"
#include "../../components/ai/AIMovementComponent.h"
#include "../Zoe.h"
#include "../Actor.h"

void Rock::SpawnRocks(Game *game, class Zathura* zathura)
{
    zathura->SetIsWaitingToThrowRocks(true);

    Vector2 cameraPos = game->GetCameraPos();

    Rock *r1 = new Rock(game, cameraPos + Vector2(144, 90), zathura);
    Rock *r2 = new Rock(game, cameraPos + Vector2(306, 90), zathura);
    Rock *r3 = new Rock(game, cameraPos + Vector2(464, 90), zathura);

    TimerComponent *timer = zathura->GetComponent<TimerComponent>();

    const float delayBetweenRocks = game->GetConfig()->Get<float>("ZATHURA.ROCK_DELAY");

    timer->AddTimer(delayBetweenRocks, [r1, game]() {
        Vector2 target = game->GetZoe()->GetCenter();
        r1->ThrowRock(target);
    });

    timer->AddTimer(2.f * delayBetweenRocks, [r2, game]() {
        Vector2 target = game->GetZoe()->GetCenter();
        r2->ThrowRock(target);
    });

    timer->AddTimer(3.f * delayBetweenRocks, [r3, game, zathura]() {
        Vector2 target = game->GetZoe()->GetCenter();
        r3->ThrowRock(target);
        
        if (zathura != nullptr) //zathura might die.
        {
            zathura->SetIsWaitingToThrowRocks(false);
        }
    });
};

Rock::Rock(
    Game* game, Vector2 position, Actor* zathura
): Projectile(game, position, zathura, 10.f)
{
    const std::string spriteSheetPath = "../assets/Sprites/Enemies/Zathura/Rock/texture.png";
    const std::string spriteSheetData = "../assets/Sprites/Enemies/Zathura/Rock/texture.json";

    mRigidBodyComponent = new RigidBodyComponent(this, 1.f, 0.f, false);
    
    mColliderComponent = new AABBColliderComponent(
        this,
        23, 24,
        17, 18,
        ColliderLayer::EnemyProjectile);
    
    mColliderComponent->SetIgnoreLayers({
        ColliderLayer::Enemy,
        ColliderLayer::EnemyProjectile,
        ColliderLayer::Quasar,
        ColliderLayer::PlayerAttack,
        ColliderLayer::SithAttack1,
        ColliderLayer::SithAttack2,
        ColliderLayer::Torch,
        ColliderLayer::Items,
        ColliderLayer::Shuriken,
        ColliderLayer::SpearTip,
        ColliderLayer::SpearBlock,
        ColliderLayer::Spikes,
        ColliderLayer::Blocks,
        ColliderLayer::SpikesBlock,
        ColliderLayer::Portal,
        ColliderLayer::Fireball,
        ColliderLayer::Nevasca,
        ColliderLayer::Zathura
    }, IgnoreOption::IgnoreResolution);

    mDrawAnimatedComponent = new DrawAnimatedComponent(
        this,
        spriteSheetPath,
        spriteSheetData,
        nullptr, // could use a lambda here too
        static_cast<int>(DrawLayerPosition::Player) - 10);

    mDrawAnimatedComponent->AddAnimation("joining", 0, 13, false);
    mDrawAnimatedComponent->AddAnimation("flying", {13});
    mDrawAnimatedComponent->SetAnimation("joining");
    
    SetBehaviorState(BehaviorState::Idle);

    SetPosition(position - GetHalfSize());

    mTimerComponent = new TimerComponent(this);
}

void Rock::ManageAnimations()
{
    switch (mBehaviorState)
    {
        case BehaviorState::Moving:
            mDrawAnimatedComponent->SetAnimation("flying");
            mDrawAnimatedComponent->SetAnimFPS(1.f);
            break;

        case BehaviorState::Idle:
            mDrawAnimatedComponent->SetAnimation("joining");
            mDrawAnimatedComponent->SetAnimFPS(8.f);
            break;
    }
}

void Rock::OnUpdate(float deltaTime)
{
    if (mBehaviorState == BehaviorState::Dying) {
        SetState(ActorState::Destroy);
        return;
    }

    Projectile::OnUpdate(deltaTime);
}

void Rock::ThrowRock(const Vector2& target)
{
    float speed = mGame->GetConfig()->Get<float>("ZATHURA.ROCK_SPEED");
    Fire(Vector2::Normalize(target - GetCenter()), speed);
    SetBehaviorState(BehaviorState::Moving);
}