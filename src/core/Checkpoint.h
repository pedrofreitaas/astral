#pragma once

#include "../libs/Math.h"

class Checkpoint {
    Checkpoint(const Vector2 &pos) : position(pos) {}

    protected:
        friend class Zoe;
        Vector2 position;
};