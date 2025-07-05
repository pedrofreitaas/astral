cmake_minimum_required(VERSION 3.26)
# Configurando o padrão c++
set(CMAKE_CXX_STANDARD 17)

# Configurando o nome e executável do projeto
set(PROJECT_NAME "tp4-smb-1-2")
project(${PROJECT_NAME})

# Use PkgConfig to find SDL2 libraries
find_package(PkgConfig REQUIRED)

# Find SDL2 with specific version
pkg_check_modules(SDL2 REQUIRED sdl2)
pkg_check_modules(SDL2_IMAGE REQUIRED SDL2_image)
pkg_check_modules(SDL2_TTF REQUIRED SDL2_ttf)
pkg_check_modules(SDL2_MIXER REQUIRED SDL2_mixer)

add_executable(${PROJECT_NAME}
        Source/Main.cpp
        Source/Math.cpp
        Source/Random.cpp
        Source/Actors/Actor.cpp
        Source/Components/Component.cpp
        Source/Game.cpp
        Source/Components/DrawComponents/DrawComponent.cpp
        Source/Components/DrawComponents/DrawComponent.h
        Source/Components/RigidBodyComponent.cpp
        Source/Components/RigidBodyComponent.h
        Source/Components/ColliderComponents/CircleColliderComponent.cpp
        Source/Components/ColliderComponents/CircleColliderComponent.h
        Source/Json.h
        Source/CSV.h
        Source/Components/DrawComponents/DrawSpriteComponent.cpp
        Source/Components/DrawComponents/DrawSpriteComponent.h
        Source/Components/DrawComponents/DrawPolygonComponent.cpp
        Source/Components/DrawComponents/DrawPolygonComponent.h
        Source/Actors/Punk.cpp
        Source/Actors/Punk.h
        Source/Components/ColliderComponents/AABBColliderComponent.cpp
        Source/Components/ColliderComponents/AABBColliderComponent.h
        Source/Components/DrawComponents/DrawAnimatedComponent.cpp
        Source/Components/DrawComponents/DrawAnimatedComponent.h
        Source/Actors/Block.cpp
        Source/Actors/Block.h
        Source/Actors/Spawner.cpp
        Source/Actors/Spawner.h
        Source/AudioSystem.cpp
        Source/AudioSystem.h
        Source/UIElements/UIScreen.h
        Source/UIElements/UIScreen.cpp
        Source/UIElements/UIFont.h
        Source/UIElements/UIFont.cpp
        Source/UIElements/UIButton.cpp
        Source/UIElements/UIButton.h
        Source/UIElements/UIText.cpp
        Source/UIElements/UIText.h
        Source/UIElements/UIElement.cpp
        Source/UIElements/UIElement.h
        Source/HUD.cpp
        Source/UIElements/UIImage.cpp
        Source/UIElements/UIImage.h
        Source/SpatialHashing.cpp
        Source/SpatialHashing.h
        Source/Actors/FSM/State.h
        Source/Actors/FSM/Patrol.cpp
        Source/Actors/FSM/Patrol.h
        Source/Actors/Enemy.cpp
        Source/Actors/Enemy.h
        Source/Actors/FSM/Chase.cpp
        Source/Actors/FSM/Chase.h
        Source/Actors/FSM/Attack.cpp
        Source/Actors/FSM/Attack.h
        Source/Actors/Projectile.cpp
        Source/Actors/Projectile.h
        Source/Actors/ProjectileEffect.cpp
        Source/Actors/ProjectileEffect.h
        Source/Actors/Portal.h
        Source/Actors/Portal.cpp
        Source/Actors/Item.h
        Source/Actors/PunkArm.h
        Source/Actors/PunkArm.cpp
        Source/Actors/Weapon.h
        Source/Actors/Weapon.cpp
)

# Link libraries using PkgConfig variables
target_include_directories(${PROJECT_NAME} PRIVATE ${SDL2_INCLUDE_DIRS} ${SDL2_IMAGE_INCLUDE_DIRS} ${SDL2_TTF_INCLUDE_DIRS} ${SDL2_MIXER_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} PRIVATE ${SDL2_LIBRARIES} ${SDL2_IMAGE_LIBRARIES} ${SDL2_TTF_LIBRARIES} ${SDL2_MIXER_LIBRARIES})
target_compile_options(${PROJECT_NAME} PRIVATE ${SDL2_CFLAGS_OTHER})