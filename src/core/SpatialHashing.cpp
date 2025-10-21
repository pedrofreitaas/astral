//
// Created by Lucas N. Ferreira on 29/05/25.
// Deeply modified and enhanced by Pedro O.
//

#include "SpatialHashing.h"
#include <SDL.h>
#include "../actors/Tile.h"
#include "../libs/Math.h"
#include "../actors/Actor.h"

SpatialHashing::SpatialHashing(int cellSize, int width, int height)
    : mCellSize(cellSize), mWidth(width), mHeight(height)
{
    int cols = (width + cellSize - 1) / cellSize;
    int rows = (height + cellSize - 1) / cellSize;

    mGrid.resize(rows, std::vector<std::vector<Actor *>>(cols));
    mCellTypes.resize(rows, std::vector<CellType>(cols, CellType::Empty));
}

SpatialHashing::~SpatialHashing()
{
    // Delete all actors
    for (auto &row : mGrid)
    {
        for (auto &cell : row)
        {
            while (!cell.empty())
            {
                delete cell.back(); // Assuming ownership of actors
            }

            cell.clear();
        }
    }

    mGrid.clear();
    mPositions.clear();
    mCellIndices.clear();
}

void SpatialHashing::Insert(Actor *actor)
{
    // Compute positions for each vertex of the collider
    Vector2 position = actor->GetPosition();

    int col = static_cast<int>(position.x / mCellSize);
    int row = static_cast<int>(position.y / mCellSize);

    // Ensure indices are within bounds
    if (col < 0 || col >= mGrid[0].size() || row < 0 || row >= mGrid.size())
    {
        return; // Out of bounds, do not insert
    }

    // Insert collider into the grid cell
    mGrid[row][col].push_back(actor);
    mPositions[actor] = position;
    mCellIndices[actor] = std::make_pair(row, col);

    if (mCellTypes[row][col] != CellType::Tile && isTileCell(row, col))
    {
        mCellTypes[row][col] = CellType::Tile;

        if (isPlaformCell(row - 1, col))
            mCellTypes[row - 1][col] = CellType::Platform;

        if (isCornerCel(row - 1, col + 1))
            mCellTypes[row - 1][col + 1] = CellType::Corner;

        if (isCornerCel(row - 1, col - 1))
            mCellTypes[row - 1][col - 1] = CellType::Corner;

        return;
    }
}

void SpatialHashing::Remove(Actor *actor)
{
    auto it = mCellIndices.find(actor);
    if (it == mCellIndices.end())
        return;

    int row = it->second.first;
    int col = it->second.second;

    // Remove the collider from the grid cell
    auto &cell = mGrid[row][col];
    cell.erase(std::remove(cell.begin(), cell.end(), actor), cell.end());

    // Remove from positions and indices maps
    mPositions.erase(actor);
    mCellIndices.erase(it);
    
    if (mCellTypes[row][col] == CellType::Tile && !isTileCell(row, col)) // if it was a tile cell, but it's no longer.
    {
        if (row <= 1)
            return;
        // there can be only one tile per block.
        // above tile is no longer a platform
        // side corners are no longer corners if they were corners for the removed tile only.

        mCellTypes[row - 1][col] = CellType::Empty;

        if (col > 1 && mCellTypes[row-1][col-1] == CellType::Corner && !isCornerCel(row-1, col-1))
        {
            mCellTypes[row - 1][col - 1] = CellType::Empty;
        }

        if (col < mGrid[0].size() && mCellTypes[row-1][col+1] == CellType::Corner && !isCornerCel(row-1, col+1))
        {
            mCellTypes[row-1][col+1] = CellType::Empty;
        }
    }
}

void SpatialHashing::Reinsert(Actor *actor)
{
    Remove(actor);
    Insert(actor);
}

std::vector<Actor *> SpatialHashing::Query(const Vector2 &position, const int range) const
{
    std::vector<Actor *> results;

    int col = static_cast<int>(position.x / mCellSize);
    int row = static_cast<int>(position.y / mCellSize);

    // Ensure indices are within bounds
    if (col < 0 || col >= mGrid[0].size() || row < 0 || row >= mGrid.size())
    {
        return results; // Out of bounds
    }

    // Check the surrounding cells
    for (int r = row - range; r <= row + range; ++r)
    {
        for (int c = col - range; c <= col + range; ++c)
        {
            if (r < 0 || r >= mGrid.size() || c < 0 || c >= mGrid[0].size())
            {
                continue; // Skip out of bounds cells
            }

            const auto &cell = mGrid[r][c];
            results.insert(results.end(), cell.begin(), cell.end());
        }
    }

    return results;
}

std::vector<AABBColliderComponent *> SpatialHashing::QueryColliders(const Vector2 &position, const int range) const
{
    std::vector<AABBColliderComponent *> results;

    std::vector<Actor *> actors = Query(position, range);
    for (Actor *actor : actors)
    {
        auto collider = actor->GetComponent<AABBColliderComponent>();
        if (collider)
        {
            results.push_back(collider);
        }
    }

    return results;
}

std::vector<Actor *> SpatialHashing::QueryOnCamera(const Vector2 &cameraPosition,
                                                   const float screenWidth,
                                                   const float screenHeight,
                                                   const float extraRadius) const
{
    std::vector<Actor *> results;

    // Get the camera vertices
    Vector2 topLeft = Vector2(cameraPosition.x - extraRadius, cameraPosition.y - extraRadius);
    Vector2 bottomRight = Vector2(cameraPosition.x + screenWidth + extraRadius, cameraPosition.y + screenHeight + extraRadius);

    // Calculate the grid cells that the camera covers
    int startCol = static_cast<int>(topLeft.x / mCellSize);
    int startRow = static_cast<int>(topLeft.y / mCellSize);
    int endCol = static_cast<int>(bottomRight.x / mCellSize);
    int endRow = static_cast<int>(bottomRight.y / mCellSize);

    // Ensure indices are within bounds
    startCol = std::max(0, startCol);
    startRow = std::max(0, startRow);
    endCol = std::min(static_cast<int>(mGrid[0].size()) - 1, endCol);
    endRow = std::min(static_cast<int>(mGrid.size()) - 1, endRow);

    // Check the cells within the camera bounds
    for (int r = startRow; r <= endRow; ++r)
    {
        for (int c = startCol; c <= endCol; ++c)
        {
            const auto &cell = mGrid[r][c];
            results.insert(results.end(), cell.begin(), cell.end());
        }
    }

    return results;
}

std::vector<Cell> SpatialHashing::findPath(const std::vector<std::vector<CellType>>& grid, Cell start, Cell end, int maxJumpHeight) const {
    int rows = grid.size(), cols = grid[0].size();
    
    auto heuristic = [&](Cell a, Cell b) {
        return std::abs(a.row - b.row) + std::abs(a.col - b.col);
    };
    
    auto isValid = [&](int r, int c) {
        return r >= 0 && r < rows && c >= 0 && c < cols 
               && (grid[c][r] != CellType::Tile);
    };
    
    auto getCellCost = [&](int r, int c) {
        CellType type = grid[c][r];
        if (type == CellType::Platform) return 1.0f;
        if (type == CellType::Corner) return 1.5f;
        return 4.0f; // Empty
    };
    
    std::priority_queue<Node, std::vector<Node>, std::greater<Node>> open;
    std::unordered_map<Cell, Cell, CellHash> cameFrom;
    std::unordered_map<Cell, float, CellHash> gScore;
    std::unordered_map<Cell, int, CellHash> upwardCount;
    
    open.push({start, (float)heuristic(start, end), 0});
    gScore[start] = 0;
    upwardCount[start] = 0;
    
    int dirs[4][2] = {{0,-1}, {0,1}, {-1,0}, {1,0}};
    
    while (!open.empty()) {
        Node curr = open.top();
        open.pop();
        
        if (curr.pos == end) {
            std::vector<Cell> path;
            Cell c = end;
            while (!(c == start)) {
                path.push_back(c);
                c = cameFrom[c];
            }
            path.push_back(start);
            std::reverse(path.begin(), path.end());
            return path;
        }
        
        for (int i = 0; i < 4; i++) {
            int nr = curr.pos.row + dirs[i][0], nc = curr.pos.col + dirs[i][1];
            if (!isValid(nr, nc)) continue;
            
            Cell neighbor = {nr, nc};
            int newUpwardSteps = upwardCount[curr.pos];
            bool movingUp = (dirs[i][1] == -1);
            
            if (movingUp) {
                newUpwardSteps++;
                if (newUpwardSteps > maxJumpHeight) continue;
            } else if (grid[nc][nr] == CellType::Platform) {
                newUpwardSteps = 0;
            }
            
            float newG = curr.g + getCellCost(nr, nc);
            
            if (!gScore.count(neighbor) || newG < gScore[neighbor] || 
                (newG == gScore[neighbor] && newUpwardSteps < upwardCount[neighbor])) {
                gScore[neighbor] = newG;
                upwardCount[neighbor] = newUpwardSteps;
                cameFrom[neighbor] = curr.pos;
                open.push({neighbor, newG + heuristic(neighbor, end), newG});
            }
        }
    }
    
    return {};
}

std::vector<Vector2> SpatialHashing::GetPath(
    Actor *targetActor, const Vector2 &end) const
{
    RigidBodyComponent *rb = targetActor->GetComponent<RigidBodyComponent>();

    if (!rb)
    {
        SDL_Log("SpatialHashing::GetPath: Actor missing RigidBody component.");
        return {};
    }

    bool gravityApplies = rb->GetApplyGravity();
    Vector2 actorPos = targetActor->GetCenter();

    Cell start = {static_cast<int>(actorPos.x / mCellSize), static_cast<int>(actorPos.y / mCellSize)};
    Cell endCell = {static_cast<int>(end.x / mCellSize), static_cast<int>(end.y / mCellSize)};

    const auto& cellTypes = mCellTypes;

    std::vector<Cell> cells = findPath(cellTypes, start, endCell);
    std::vector<Vector2> positions;

    for (auto cell : cells) {
        positions.push_back(
            Vector2(
                cell.row * mCellSize + mCellSize/2.f, 
                cell.col * mCellSize + mCellSize/2.f));
    }

    return positions;
}

void SpatialHashing::Draw(SDL_Renderer *renderer, const Vector2 &cameraPosition, float screenWidth, float screenHeight)
{
    // Draw grid lines
    SDL_SetRenderDrawColor(renderer, 200, 200, 200, 255);

    int cols = (mWidth + mCellSize - 1) / mCellSize;
    int rows = (mHeight + mCellSize - 1) / mCellSize;

    for (int c = 0; c <= cols; ++c)
    {
        int x = c * mCellSize - static_cast<int>(cameraPosition.x);
        if (x >= 0 && x <= screenWidth)
        {
            SDL_RenderDrawLine(renderer, x, 0, x, static_cast<int>(screenHeight));
        }
    }

    for (int r = 0; r <= rows; ++r)
    {
        int y = r * mCellSize - static_cast<int>(cameraPosition.y);
        if (y >= 0 && y <= screenHeight)
        {
            SDL_RenderDrawLine(renderer, 0, y, static_cast<int>(screenWidth), y);
        }
    }

    for (int r = 0; r < rows; ++r)
    {
        for (int c = 0; c < cols; ++c)
        {
            if (mCellTypes[r][c] == CellType::Tile)
            {
                int x = c * mCellSize + mCellSize / 2 - static_cast<int>(cameraPosition.x) - 5;
                int y = r * mCellSize + mCellSize / 2 - static_cast<int>(cameraPosition.y) - 5;
                SDL_Rect rect = {x, y, 10, 10};
                SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
                SDL_RenderFillRect(renderer, &rect);
            }

            else if (mCellTypes[r][c] == CellType::Platform)
            {
                int x = c * mCellSize + mCellSize / 2 - static_cast<int>(cameraPosition.x) - 5;
                int y = r * mCellSize + mCellSize / 2 - static_cast<int>(cameraPosition.y) - 5;
                SDL_Rect rect = {x, y, 10, 10};
                SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
                SDL_RenderFillRect(renderer, &rect);
            }

            else if (mCellTypes[r][c] == CellType::Corner)
            {
                int x = c * mCellSize + mCellSize / 2 - static_cast<int>(cameraPosition.x) - 5;
                int y = r * mCellSize + mCellSize / 2 - static_cast<int>(cameraPosition.y) - 5;
                SDL_Rect rect = {x, y, 10, 10};
                SDL_SetRenderDrawColor(renderer, 255, 165, 0, 255);
                SDL_RenderFillRect(renderer, &rect);

                int auxRow = r + 1;
                while (mCellTypes[r][c] != CellType::Platform && false)
                {
                    int y = auxRow * mCellSize + mCellSize / 2 - static_cast<int>(cameraPosition.y) - 5;
                    SDL_Rect rect = {x, y, 10, 10};
                    SDL_RenderFillRect(renderer, &rect);

                    auxRow++;
                    if (auxRow > mGrid.size())
                        break;
                }
            }
        }
    }
}

bool SpatialHashing::isTileCell(int row, int col)
{
    // Ensure indices are within bounds
    if (col < 0 || col >= mGrid[0].size() || row < 0 || row >= mGrid.size())
    {
        return false; // Out of bounds
    }

    std::vector<Actor *> actors = mGrid[row][col];

    if (actors.empty())
        return false;

    return std::any_of(actors.begin(), actors.end(), [](Actor *a)
                       { return dynamic_cast<Tile *>(a) != nullptr; });
}

bool SpatialHashing::isPlaformCell(int row, int col)
{
    // Ensure indices are within bounds
    if (col < 0 || col >= mGrid[0].size() || row < 0 || row >= mGrid.size() - 1)
    {
        return false; // Out of bounds
    }

    return isTileCell(row + 1, col) && !isTileCell(row, col);
}

bool SpatialHashing::isCornerCel(int row, int col)
{
    // Ensure indices are within bounds
    if (col < 1 || col >= mGrid[0].size() - 1 || row < 0 || row >= mGrid.size() - 1)
    {
        return false; // Out of bounds
    }

    return !isTileCell(row, col) && !isPlaformCell(row, col) &&
           (isPlaformCell(row, col + 1) || isPlaformCell(row, col - 1));
}

bool SpatialHashing::isEmptyCell(int row, int col)
{
    // Ensure indices are within bounds
    if (col < 0 || col >= mGrid[0].size() || row < 0 || row >= mGrid.size())
    {
        return false; // Out of bounds
    }

    return !isTileCell(row, col) && !isPlaformCell(row, col) && !isCornerCel(row, col);
}