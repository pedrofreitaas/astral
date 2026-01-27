cmake_minimum_required(VERSION 3.26)

project(astral)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# --- 1. Library Paths ---
set(SDL2_ROOT        "C:/libs/SDL2/x86_64-w64-mingw32")
set(SDL2_IMAGE_ROOT  "C:/libs/SDL2_image/x86_64-w64-mingw32")
set(SDL2_TTF_ROOT    "C:/libs/SDL2_ttf/x86_64-w64-mingw32")
set(SDL2_MIXER_ROOT  "C:/libs/SDL2_mixer/x86_64-w64-mingw32")

# --- 2. Source Files ---
add_executable(astral
    src/Main.cpp
    src/libs/Math.h
    src/libs/Math.cpp
    src/libs/Random.h
    src/libs/Random.cpp
    src/actors/Actor.cpp
    src/components/Component.cpp
    src/core/Config.h
    src/core/Config.cpp
    src/core/Game.h
    src/core/Game.cpp
    src/core/ScenesLoading.cpp
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
    src/actors/ZoeFireball.cpp
    src/actors/ZoeFireball.h
    src/actors/Nevasca.cpp
    src/actors/Nevasca.h
    src/actors/Ventania.h
    src/actors/Ventania.cpp
    src/components/collider/AABBColliderComponent.cpp
    src/components/collider/AABBColliderComponent.h
    src/components/draw/DrawAnimatedComponent.cpp
    src/components/draw/DrawAnimatedComponent.h
    src/components/ai/AIMovementComponent.cpp
    src/components/ai/AIMovementComponent.h
    src/actors/Snow.h
    src/actors/Snow.cpp
    src/actors/Tile.cpp
    src/actors/Tile.h
    src/actors/Star.cpp
    src/actors/Star.h
    src/actors/MapObject.h
    src/actors/MapObject.cpp
    src/actors/Enemy.cpp
    src/actors/Enemy.h
    src/actors/traps/Spikes.h
    src/actors/traps/Spikes.cpp
    src/actors/traps/Spear.h
    src/actors/traps/Spear.cpp
    src/actors/traps/Shuriken.h
    src/actors/traps/Shuriken.cpp
    src/actors/enemies/Zod.cpp
    src/actors/enemies/Zod.h
    src/actors/enemies/ZodProjectile.cpp
    src/actors/enemies/ZodProjectile.h
    src/actors/enemies/Sith.cpp
    src/actors/enemies/Sith.h
    src/actors/enemies/Quasar.cpp
    src/actors/enemies/Quasar.h
    src/actors/Portal.cpp
    src/actors/Portal.h
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
    src/actors/Projectile.h
    src/actors/Projectile.cpp
    src/actors/Collider.cpp
    src/actors/Collider.h
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
    src/core/Steps.cpp
    src/components/TimerComponent.h
)

# --- 3. Include Directories ---
target_include_directories(astral PRIVATE
    ${SDL2_ROOT}/include
    ${SDL2_ROOT}/include/SDL2

    ${SDL2_IMAGE_ROOT}/include
    ${SDL2_IMAGE_ROOT}/include/SDL2

    ${SDL2_TTF_ROOT}/include
    ${SDL2_TTF_ROOT}/include/SDL2

    ${SDL2_MIXER_ROOT}/include
    ${SDL2_MIXER_ROOT}/include/SDL2
)

# --- 4. Link Directories ---
target_link_directories(astral PRIVATE
    ${SDL2_ROOT}/lib
    ${SDL2_IMAGE_ROOT}/lib
    ${SDL2_TTF_ROOT}/lib
    ${SDL2_MIXER_ROOT}/lib
)

# --- 5. Link Libraries & Static Config ---
target_link_libraries(astral PRIVATE
    mingw32
    SDL2main
    SDL2
    SDL2_image
    SDL2_ttf
    SDL2_mixer
)

if(MINGW)
    target_link_options(astral PRIVATE 
        -static-libgcc 
        -static-libstdc++ 
        # -mwindows # Hides the console window
    )
endif()

# --- 6. Auto-Copy DLLs ---
add_custom_command(TARGET astral POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E echo "Copying DLLs to build directory..."
    
    # SDL2
    COMMAND ${CMAKE_COMMAND} -E copy_if_different 
        "${SDL2_ROOT}/bin/SDL2.dll" 
        $<TARGET_FILE_DIR:astral>
        
    # SDL2 Image
    COMMAND ${CMAKE_COMMAND} -E copy_if_different 
        "${SDL2_IMAGE_ROOT}/bin/SDL2_image.dll" 
        $<TARGET_FILE_DIR:astral>
        
    # SDL2 TTF
    COMMAND ${CMAKE_COMMAND} -E copy_if_different 
        "${SDL2_TTF_ROOT}/bin/SDL2_ttf.dll" 
        $<TARGET_FILE_DIR:astral>
        
    # SDL2 Mixer
    COMMAND ${CMAKE_COMMAND} -E copy_if_different 
        "${SDL2_MIXER_ROOT}/bin/SDL2_mixer.dll" 
        $<TARGET_FILE_DIR:astral>
)

add_custom_command(TARGET astral POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    "${CMAKE_SOURCE_DIR}/assets"
    "$<TARGET_FILE_DIR:astral>/assets"
)