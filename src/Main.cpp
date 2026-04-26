// ----------------------------------------------------------------
// From Game Programming in C++ by Sanjay Madhav
// Copyright (C) 2017 Sanjay Madhav. All rights reserved.
//
// Released under the BSD License
// See LICENSE in root directory for full details.
// ----------------------------------------------------------------

#include "core/Game.h"
#define SDL_MAIN_HANDLED

#ifdef __EMSCRIPTEN__
#include <emscripten.h>

static Game* gGame = nullptr;

void emscripten_loop()
{
    gGame->ProcessInput();
    gGame->UpdateGame();
    gGame->GenerateOutput();
}
#endif

int main(int argc, char** argv)
{
    Game game = Game();
    bool success = game.Initialize();

    if (!success) return 1;

#ifdef __EMSCRIPTEN__
    gGame = &game;
    emscripten_set_main_loop(emscripten_loop, 0, 1);
#else
    game.RunLoop();
    game.Shutdown();
#endif
    return 0;
}
