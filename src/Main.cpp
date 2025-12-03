// ----------------------------------------------------------------
// From Game Programming in C++ by Sanjay Madhav
// Copyright (C) 2017 Sanjay Madhav. All rights reserved.
//
// Released under the BSD License
// See LICENSE in root directory for full details.
// ----------------------------------------------------------------

#include "core/Game.h"
#define SDL_MAIN_HANDLED

//Screen dimension constants
// const int SCREEN_WIDTH = 1920;
// const int SCREEN_HEIGHT = 1080;

int main(int argc, char** argv)
{
    Game game = Game();
    bool success = game.Initialize();

    if (!success) throw std::runtime_error("Failed to initialize game.");

    game.RunLoop();

    game.Shutdown();
    return 0;
}
