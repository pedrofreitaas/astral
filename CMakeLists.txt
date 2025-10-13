cmake_minimum_required(VERSION 3.26)
# Configurando o padrão c++
set(CMAKE_CXX_STANDARD 17)

# Configurando o nome e executável do projeto
set(PROJECT_NAME "astral")
project(${PROJECT_NAME})

# Use PkgConfig to find SDL2 libraries
find_package(PkgConfig REQUIRED)

# Find SDL2 with specific version
pkg_check_modules(SDL2 REQUIRED sdl2)
pkg_check_modules(SDL2_IMAGE REQUIRED SDL2_image)
pkg_check_modules(SDL2_TTF REQUIRED SDL2_ttf)
pkg_check_modules(SDL2_MIXER REQUIRED SDL2_mixer)

add_executable(${PROJECT_NAME}
        src/Main.cpp
        src/libs/Math.h        
        src/libs/Math.cpp
        src/libs/Random.h
        src/libs/Random.cpp
        src/actors/Actor.cpp
        src/components/Component.cpp
        src/core/Game.h
        src/core/Game.cpp
        src/components/draw/DrawComponent.cpp
        src/components/draw/DrawComponent.h
        src/components/draw/DrawTileComponent.cpp
        src/components/draw/DrawTileComponent.h
        src/components/RigidBodyComponent.cpp
        src/components/RigidBodyComponent.h
        src/components/collider/CircleColliderComponent.cpp
        src/components/collider/CircleColliderComponent.h
        src/libs/Json.h
        src/components/draw/DrawSpriteComponent.cpp
        src/components/draw/DrawSpriteComponent.h
        src/actors/Zoe.cpp
        src/actors/Zoe.h
        src/components/collider/AABBColliderComponent.cpp
        src/components/collider/AABBColliderComponent.h
        src/components/draw/DrawAnimatedComponent.cpp
        src/components/draw/DrawAnimatedComponent.h
        src/components/ai/AIMovementComponent.cpp
        src/components/ai/AIMovementComponent.h
        src/actors/Tile.cpp
        src/actors/Tile.h
        src/actors/Star.cpp
        src/actors/Star.h
        src/actors/MapObject.h
        src/actors/MapObject.cpp
        src/actors/Enemy.cpp
        src/actors/Enemy.h
        src/core/AudioSystem.cpp
        src/core/AudioSystem.h
        src/ui/UIScreen.h
        src/ui/UIScreen.cpp
        src/ui/UIFont.h
        src/ui/UIFont.cpp
        src/ui/UIButton.cpp
        src/ui/UIButton.h
        src/ui/UIText.cpp
        src/ui/UIText.h
        src/ui/UIElement.cpp
        src/ui/UIElement.h
        src/ui/UICursor.cpp
        src/ui/UICursor.h
        src/core/HUD.h
        src/core/HUD.cpp
        src/ui/UIImage.cpp
        src/ui/UIImage.h
        src/core/SpatialHashing.cpp
        src/core/SpatialHashing.h
        src/actors/Projectile.cpp
        src/actors/Projectile.h
        src/actors/Item.h
        src/actors/Item.cpp
        src/ui/DialogueSystem.cpp
        src/ui/DialogueSystem.h
        src/core/Tileset.h
        src/core/Tileset.cpp
        src/core/Map.h
        src/core/Map.cpp
        src/core/Cutscene.h
        src/core/Cutscene.cpp
)

# Link libraries using PkgConfig variables
target_include_directories(${PROJECT_NAME} PRIVATE ${SDL2_INCLUDE_DIRS} ${SDL2_IMAGE_INCLUDE_DIRS} ${SDL2_TTF_INCLUDE_DIRS} ${SDL2_MIXER_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} PRIVATE ${SDL2_LIBRARIES} ${SDL2_IMAGE_LIBRARIES} ${SDL2_TTF_LIBRARIES} ${SDL2_MIXER_LIBRARIES})
target_compile_options(${PROJECT_NAME} PRIVATE ${SDL2_CFLAGS_OTHER})