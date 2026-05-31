#include <algorithm>
#include <utility>
#include <stdexcept>
#include "Cutscene.h"
#include "Game.h"
#include "../actors/Star.h"
#include "../actors/Zoe.h"
#include "../ui/DialogueSystem.h"
#include "../ui/UIAnimation.h"
#include "../core/HUD.h"
#include "../actors/Father.h"
#include "../actors/Portal.h"

MoveStep::MoveStep(
    class Game* game, 
    std::function<Actor*()> targetActorFunc,
    std::function<Vector2()> getTargetPosFunc,
    float speed, bool spin, float maxTime, bool blockGravity): 
    Step(game, maxTime), mSpeed(speed), 
    mGetTargetActor(std::move(targetActorFunc)), 
    mGetTargetPos(std::move(getTargetPosFunc)),
    mTargetPos(Vector2::Zero), mSpinAngle(0.0f), 
    mSpin(spin), mRemoveGravityIfNeeded(blockGravity)
{
}

void MoveStep::PreUpdate()
{
    mTargetPos = mGetTargetPos();
    Actor *mTargetActor = mGetTargetActor();

    if (!mTargetActor)
    {
        throw std::runtime_error("MoveStep target Actor is null");
    }

    RigidBodyComponent *rb = mTargetActor->GetComponent<RigidBodyComponent>();
    if (!rb)
    {
        throw std::runtime_error("MoveStep target Actor lost its RigidBodyComponent");
    }

    if (!rb->GetApplyGravity() && mRemoveGravityIfNeeded)
    {
        throw std::runtime_error("MoveStep was set to block gravity but target Actor's RigidBodyComponent already has gravity disabled");
    }

    if (rb->GetApplyGravity() && mRemoveGravityIfNeeded)
    {
        rb->SetApplyGravity(false);
    }
}

void MoveStep::SetComplete(bool v)
{
    Step::SetComplete(v);

    Actor *mTargetActor = mGetTargetActor();
    if (!mTargetActor)
    {
        throw std::runtime_error("MoveStep target Actor is null");
    }

    RigidBodyComponent *rb = mTargetActor->GetComponent<RigidBodyComponent>();
    if (!rb)
    {
        throw std::runtime_error("MoveStep target Actor lost its RigidBodyComponent");
    }

    if (mRemoveGravityIfNeeded)
        rb->SetApplyGravity(true);
}

void MoveStep::Update(float deltaTime)
{
    if (GetIsComplete())
        return;
    
    Step::Update(deltaTime);
    
    Actor *mTargetActor = mGetTargetActor();    
    RigidBodyComponent *rb = mTargetActor->GetComponent<RigidBodyComponent>();
    
    // Get the center position of the actor
    Vector2 actorCenterOffset = mTargetActor->GetCenter() - mTargetActor->GetPosition();
    Vector2 actorCenter = mTargetActor->GetCenter();
    
    Vector2 toTarget = mTargetPos - actorCenter;
    float distanceToTarget = toTarget.Length();
    
    // Close enough to target
    if (distanceToTarget < 1e-3f)
    {
        rb->ResetVelocity();
        mTargetActor->SetPosition(mTargetPos - actorCenterOffset);
        SetComplete();
        return;
    }

    if (mSpin)
    {
        mSpinAngle += 360.f * 1.3 * deltaTime;
        if (mSpinAngle >= 360.f)
            mSpinAngle -= 360.f;
        mTargetActor->SetRotation(mSpinAngle);
    }

    Vector2 direction = toTarget;
    direction.Normalize();
    Vector2 desiredVelocity = direction * mSpeed;

    if (rb->GetApplyGravity() && mGame->GetApplyGravityScene())
    {
        rb->ResetVelocityX();
        rb->ApplyImpulse(Vector2(desiredVelocity.x - rb->GetVelocity().x, 0.f));
    }
    else {
        rb->ResetVelocity();
        rb->ApplyImpulse(desiredVelocity);
    }

    // Check if we would overshoot the target in this frame
    if (distanceToTarget < desiredVelocity.Length() * deltaTime)
    {
        // Snap to target and complete step
        rb->ResetVelocity();
        mTargetActor->SetPosition(mTargetPos - actorCenterOffset);
        SetComplete();
    }
    
    // check if movement resulted in getting out of the camera
    if (!mGame->ActorOnCamera(mTargetActor))
    {
        rb->ResetVelocity();
        SetComplete();
    }
}

void SpawnStep::Update(float deltaTime)
{
    if (GetIsComplete())
        return;

    Step::Update(deltaTime);

    Actor *newActor = nullptr;

    if (mActorType == ActorType::Star)
    {
        newActor = new Star(mGame, mPosition);
    }

    else if (mActorType == ActorType::Father)
    {
        newActor = new Father(mGame, mPosition);
    }

    else if (mActorType == ActorType::Portal)
    {
        newActor = new Portal(mGame, mPosition);
    }

    if (newActor)
    {
        newActor->SetRotation(mRotation);
        SetComplete();
    }
    else
    {
        throw std::runtime_error("SpawnStep failed to create actor of type: " +
                                 std::to_string(static_cast<int>(mActorType)));
    }
}

void DialogueStep::Initialize()
{
    if (!mSpeaker.empty())
    {
        mGame->GetDialogueSystem()->StartDialogueWithSpeaker(
            mSpeaker,
            mMessages,
            [this](){
                mGame->SetGamePlayState(Game::GamePlayState::PlayingCutscene);
                SetComplete();
            });
        
        return;
    }

    mGame->GetDialogueSystem()->StartDialogue(
        mMessages,
        [this](){
            mGame->SetGamePlayState(Game::GamePlayState::PlayingCutscene);
            SetComplete();
        });
}

void SpawnJoystickButtonStep::OnProcessInput(const std::vector<SDL_Event>& events) {
    if (GetIsComplete() || !mAnimation) return;

    SDL_GameController* controller = mGame->GetController();

    if (mButton == Button::RT && SDL_GameControllerGetAxis(controller, SDL_CONTROLLER_AXIS_TRIGGERRIGHT) > 0) {
        mGame->GetHUD()->RemoveAnimation(mAnimation);
        SetComplete();  
        return;
    }

    if (mButton != Button::LT && SDL_GameControllerGetAxis(controller, SDL_CONTROLLER_AXIS_TRIGGERLEFT) > 0) {
        mGame->GetHUD()->RemoveAnimation(mAnimation);
        SetComplete();  
        return;
    }

    for (const SDL_Event& event : events) {
        if (event.type == SDL_CONTROLLERBUTTONDOWN) {
            if (event.cbutton.button == GetSDLButton(mButton))
            {
                mGame->GetHUD()->RemoveAnimation(mAnimation);
                SetComplete();
            }

            break;
        }
    }
}

void SpawnJoystickButtonStep::Update(float deltaTime) {
    if (GetIsComplete()) return;

    if (!mAnimation) {
        Zoe* zoe = mGame->GetZoe();
        Vector2 screenPos = zoe->GetCenter() - mGame->GetCameraPos();
        screenPos += Vector2(-8.f, -35.f);

        std::pair<int, int> animRange = joystickButtonsSpriteMapping.at(mButton);

        mAnimation = mGame->GetHUD()->AddAnimation(
            "../assets/Sprites/Joystick/texture.png",
            "../assets/Sprites/Joystick/texture.json",
            screenPos, Vector2::Zero, 4.f,
            animRange.first, animRange.second, true);
    }
}

void LaunchFireballStep::Update(float deltaTime) {
    if (GetIsComplete())
        return;
    
    Step::Update(deltaTime);

    mGame->GetZoe()->OnFireballPressed();

    mGame->GetZoe()->GetComponent<TimerComponent>()->AddTimer(1.0f, [this]() {
        mGame->GetZoe()->OnFireballReleased();
        SetComplete();
    });
}

void FireNevascaStep::PreUpdate() {
    Zoe* zoe = mGame->GetZoe();
    zoe->SetMana(mGame->GetConfig()->Get<float>("ZOE.MAX_MANA"));
    zoe->OnNevascaPressed();
}

void FireNevascaStep::Update(float deltaTime) {
    if (GetIsComplete()) return;
    Step::Update(deltaTime);
    if (GetIsComplete())
        mGame->GetZoe()->OnNevascaReleased();
}

void SoundStep::Update(float deltaTime)
{
    if (GetIsComplete())
        return;
    
    Step::Update(deltaTime);

    mGame->GetAudio()->PlaySound(mSoundFile, mLoop);
    SetComplete();
}

void ShakeStep::Update(float deltaTime)
{
    if (GetIsComplete()) return;
    mGame->SetCameraCenterToShake(mDuration, mIntensity);
    SetComplete();
}

void JumpStep::PreUpdate()
{
    Zoe* zoe = mGame->GetZoe();
    zoe->OnJumpPressed();
}

void JumpStep::SetComplete(bool v)
{
    Zoe* zoe = mGame->GetZoe();
    zoe->OnJumpReleased();
    Step::SetComplete(v);
}

void VentaniaStep::PreUpdate()
{
    Zoe* zoe = mGame->GetZoe();
    zoe->OnVentaniaPressed();
}

void VentaniaStep::SetComplete(bool v)
{
    Zoe* zoe = mGame->GetZoe();
    zoe->OnVentaniaReleased();

    Step::SetComplete(v);
}

void FreezePhysicsStep::PreUpdate()
{
    mGame->SetPhysicsFrozen(true);
}

void UnfreezePhysicsStep::PreUpdate()
{
    mGame->SetPhysicsFrozen(false);
}

void DodgeStep::PreUpdate()
{
    auto *zoe = mGame->GetZoe();
    if (!zoe) return;

    zoe->OnDodgePressed();
}

void DodgeStep::SetComplete(bool v)
{
    auto *zoe = mGame->GetZoe();
    if (!zoe) return;

    zoe->OnDodgeReleased();
    Step::SetComplete(v);
}

void BreakTileStep::PreUpdate()
{
    auto *spatialHashing = mGame->GetSpatialHashing();

    mTile = spatialHashing->GetTileAtPos(mTileCenter);

    if (mTile == nullptr)
    {
        throw std::runtime_error("BreakTileStep failed to find tile at position: (" +
                                 std::to_string(mTileCenter.x) + ", " + std::to_string(mTileCenter.y) + ")");
    }
}

void BreakTileStep::Update(float deltaTime)
{
    if (GetIsComplete())
        return;

    Step::Update(deltaTime);

    Vector2 shakeOffset = Vector2(
        Math::RandRangeInt(-3, 3),
        Math::RandRangeInt(-3, 3)
    );

    mTile->SetPosition(mTileCenter + shakeOffset);
}

void BreakTileStep::SetComplete(bool v)
{
    Step::SetComplete(v);
    mTile->Break();
    mGame->GetAudio()->PlaySound("breakTile.wav");
}