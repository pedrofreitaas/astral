//
// Created by Lucas N. Ferreira on 29/05/25.
//

#include "SpatialHashing.h"
#include <SDL.h>
#include <algorithm>

SpatialHashing::SpatialHashing(int cellSize, int width, int height)
    : mCellSize(cellSize), mWidth(width), mHeight(height)
{
    int cols = (width + cellSize - 1) / cellSize;
    int rows = (height + cellSize - 1) / cellSize;
    mGrid.resize(rows, std::vector<std::vector<Actor *>>(cols));
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
                delete cell.back();
                ; // Assuming ownership of actors
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
}

void SpatialHashing::Remove(Actor *actor)
{
    auto it = mCellIndices.find(actor);
    if (it != mCellIndices.end())
    {
        int row = it->second.first;
        int col = it->second.second;

        // Remove the collider from the grid cell
        auto &cell = mGrid[row][col];
        cell.erase(std::remove(cell.begin(), cell.end(), actor), cell.end());

        // Remove from positions and indices maps
        mPositions.erase(actor);
        mCellIndices.erase(it);
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

std::vector<Vector2> SpatialHashing::GetPath(
    Actor *targetActor, const Vector2& end
) const {
    RigidBodyComponent* rb = targetActor->GetComponent<RigidBodyComponent>();
    AABBColliderComponent* collider = targetActor->GetComponent<AABBColliderComponent>();

    if (!rb || !collider) {
        SDL_Log("SpatialHashing::GetPath: Actor missing RigidBody or Collider component.");
        return {};
    }

    bool gravityApplies = rb->GetApplyGravity();

    // implement A* from target actor center to end position
    // taking into account the collider size and gravity if applies

    return {
        targetActor->GetCenter(),
        Vector2::Lerp(targetActor->GetCenter(), end, 0.25f),
        Vector2::Lerp(targetActor->GetCenter(), end, 0.5f),
        Vector2::Lerp(targetActor->GetCenter(), end, 0.75f),
        end
    };
}

void SpatialHashing::Draw(SDL_Renderer *renderer, const Vector2& cameraPosition, float screenWidth, float screenHeight) {
    // Draw grid lines
    SDL_SetRenderDrawColor(renderer, 200, 200, 200, 255);

    int cols = (mWidth + mCellSize - 1) / mCellSize;
    int rows = (mHeight + mCellSize - 1) / mCellSize;

    for (int c = 0; c <= cols; ++c) {
        int x = c * mCellSize - static_cast<int>(cameraPosition.x);
        if (x >= 0 && x <= screenWidth) {
            SDL_RenderDrawLine(renderer, x, 0, x, static_cast<int>(screenHeight));
        }
    }

    for (int r = 0; r <= rows; ++r) {
        int y = r * mCellSize - static_cast<int>(cameraPosition.y);
        if (y >= 0 && y <= screenHeight) {
            SDL_RenderDrawLine(renderer, 0, y, static_cast<int>(screenWidth), y);
        }
    }
}