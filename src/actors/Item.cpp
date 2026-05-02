#include "./Item.h"
#include "./Zoe.h" // must be here due to circular dependency
#include "../components/draw/DrawSpriteComponent.h"
#include "../core/Cutscene.h"

Item *Item::CreateNevascaItem(Game *game, const Vector2 &position)
{
  return new Item(
      game,
      position,
      "../assets/Sprites/Zoe/Nevasca/texture.png",
      "../assets/Sprites/Zoe/Nevasca/texture.json",
      17, 17,
      [game](Item &item)
      {
        std::vector<std::unique_ptr<Step>> steps;
        
        steps.push_back(std::make_unique<MoveStep>(
            game,
            [game]() { return game->GetZoe(); },
            [game]() { return Vector2(game->GetZoe()->GetCenter().x + 32.f, game->GetZoe()->GetCenter().y); },
            80.f));

        auto spawnAnim = std::make_unique<SpawnUIAnimationStep>(game, 20, 23);
        SpawnUIAnimationStep *spawnAnimPtr = spawnAnim.get();
        steps.push_back(std::move(spawnAnim));

        steps.push_back(std::make_unique<FireNevascaStep>(game, 2.f));
        
        steps.push_back(std::make_unique<UnspawnUIAnimationStep>(game, [spawnAnimPtr]()
            { return spawnAnimPtr->GetAnimation(); }));
        
          std::vector<std::string> dialogue = {"O que e isso? Que frio..."};
        steps.push_back(std::make_unique<DialogueStep>(game, "Zoe", dialogue));
        
        game->AddCutscene("nevasca_acquisition", std::move(steps), nullptr);
        
        game->GetZoe()->SetIsNevascaAllowed(true);
        
        game->StartCutscene("nevasca_acquisition");
      },
      Button::RT,
      0, 4, 5);
}

Item::Item(
    Game *game,
    const Vector2 &position,
    std::string texturePath,
    std::string dataPath,
    int bbWidth, int bbHeight,
    PickHandler onPickCallback,
    Button button,
    int animationStartIdx, int animationEndIdx, float animFPS)
    : Actor(game),
      mOnPickCallback(onPickCallback),
      mIsPicked(false), mIsPickable(false),
      mButton(button)
{
  mRigidBodyComponent = new RigidBodyComponent(this, 1);

  mColliderComponent = new AABBColliderComponent(
      this, 0, 0, bbWidth, bbHeight, ColliderLayer::Items, true, 10);

  mDrawComponent = new DrawAnimatedComponent(
      this,
      texturePath,
      dataPath,
      nullptr);

  mDrawComponent->AddAnimation("idle", animationStartIdx, animationEndIdx);
  mDrawComponent->SetAnimation("idle");
  mDrawComponent->SetAnimFPS(animFPS);

  mButtonDrawComponent = new DrawAnimatedComponent(
      this,
      "../assets/Sprites/Joystick/texture.png",
      "../assets/Sprites/Joystick/texture.json",
      nullptr);

  mButtonDrawComponent->AddAnimation(
      "pressing",
      joystickButtonsSpriteMapping[button].first,
      joystickButtonsSpriteMapping[button].second);

  mButtonDrawComponent->SetAnimation("pressing");
  mButtonDrawComponent->SetIsVisible(false);
  mButtonDrawComponent->SetAnimFPS(4.f);
  mButtonDrawComponent->SetOffset(
      Vector2(GetHalfSize().x - 8, -20));

  SetPosition(position);
}

void Item::OnUpdate(float deltaTime)
{
  if (mIsPicked)
    Kill();

  if (mIsPickable && !mButtonDrawComponent->IsVisible())
    mButtonDrawComponent->SetIsVisible(true);

  else if (!mIsPickable && mButtonDrawComponent->IsVisible())
    mButtonDrawComponent->SetIsVisible(false);

  Vector2 playerPos = mGame->GetZoe()->GetCenter();
  float distanceToPlayerSq = (GetCenter() - playerPos).LengthSq();

  mIsPickable = distanceToPlayerSq < 2500.f; // 50 units

  Zoe *zoe = mGame->GetZoe();

  if (mIsPickable)
    zoe->BlockButton(mButton);
}

void Item::OnProcessInput(const Uint8 *keyState, const std::vector<SDL_Event> &events)
{
  if (mIsPicked || !mIsPickable)
    return;

  SDL_GameController *controller = GetGame()->GetController();

  if (mButton == Button::RT && SDL_GameControllerGetAxis(controller, SDL_CONTROLLER_AXIS_TRIGGERRIGHT) <= 0)
  {
    return;
  }

  else if (mButton == Button::LT && SDL_GameControllerGetAxis(controller, SDL_CONTROLLER_AXIS_TRIGGERLEFT) <= 0)
  {
    return;
  }

  else if (mButton != Button::RT && mButton != Button::LT /*needs to be here*/ && !SDL_GameControllerGetButton(controller, GetSDLButton(mButton)))
  {
    return;
  }

  mGame->GetAudio()->PlaySound("PickItem.wav");
  mIsPicked = true;
  if (mOnPickCallback)
    mOnPickCallback(*this);
}

void Item::Kill()
{
  SetState(ActorState::Destroy);
}