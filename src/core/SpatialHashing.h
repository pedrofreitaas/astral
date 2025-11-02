//
// Created by Lucas N. Ferreira on 29/05/25.
//

#pragma once

#include <vector>
#include <unordered_map>
#include <SDL.h>
#include <queue>
#include <unordered_map>
#include <cmath>
#include <algorithm>
#include <functional>
#include <utility>
#include "../libs/Math.h"
#include "../actors/Actor.h"

struct Cell {
    int row, col;
    bool operator==(const Cell& o) const { return row == o.row && col == o.col; }
};

struct CellHash {
    size_t operator()(const Cell& c) const { return c.row * 10000 + c.col; }
};

struct Node {
    Cell pos;
    float f, g;
    bool operator>(const Node& o) const { return f > o.f; }
};

enum class CellType // only considers tiles
{
    Empty, //blank
    Tile, //red
    Platform, //green
    Corner //orange
};

class SpatialHashing
{
public:
    SpatialHashing(int cellSize, int width, int height);
    ~SpatialHashing();

    void Insert(Actor *actor);
    void Remove(Actor *actor);
    void Reinsert(Actor *actor);

    std::vector<AABBColliderComponent *> QueryColliders(const Vector2& position, const int range = 1) const;

    std::vector<Actor*> Query(const Vector2& position, const int range = 1) const;
    std::vector<Actor*> QueryOnCamera(const Vector2& cameraPosition,
                                      const float screenWidth,
                                      const float screenHeight,
                                      const float extraRadius = 0.0f) const;

    std::vector<SDL_Rect> GetPath(
        Actor *targetActor, const Vector2& end, bool canFly = false
    ) const;

    void Draw(SDL_Renderer *renderer, const Vector2& cameraPosition, float screenWidth, float screenHeight);
   
    bool isEmptyCell(int row, int col);
    bool isTileCell(int row, int col);
    bool isPlaformCell(int row, int col);
    bool isCornerCel(int row, int col);

private:
    int mCellSize;
    int mWidth;
    int mHeight;

    std::vector<std::vector<CellType>> mCellTypes; // 2D grid of cell types
    std::vector<std::vector<std::vector<Actor*> >> mGrid; // 2D grid of colliders
    std::unordered_map<Actor*, Vector2> mPositions; // Maps collider to its position
    std::unordered_map<Actor*, std::pair<int, int>> mCellIndices; // Maps collider to its grid cell indices

    std::vector<Cell> findPath(
        const std::vector<std::vector<CellType>>& grid, 
        Cell start, Cell end,
        std::function<float(int row, int col)> getCellCost,
        std::function<bool(int row, int col)> isValid,
        std::function<int(Cell, Cell)> heuristic,
        int maxJumpHeight=4
    ) const;
};
