#include "./Tileset.h"
#include <fstream>

Tileset::Tileset(std::string jsonPath)
{
    std::ifstream file(jsonPath);
    json data = json::parse(file);

    mTileHeight = data["tileheight"];
    mTileWidth = data["tilewidth"];
    mImageHeight = data["imageheight"];
    mImageWidth = data["imagewidth"];
    mName = data["name"];

    mTiles = std::vector<Tile>();

    for (auto& element : data["tiles"].items()) {
        Tile tile;
        tile.id = element.value()["id"];
        if (element.value().contains("objectgroup")) {
            tile.mObjectGroup = element.value()["objectgroup"];
        }
        mTiles.push_back(tile);
    }
}

Tileset::~Tileset() {}

void Tileset::print()
{
    SDL_Log("Name: %s\n", mName.c_str());
    SDL_Log("Tile Size: %dx%d\n", mTileWidth, mTileHeight);
    SDL_Log("Image Size: %dx%d\n", mImageWidth, mImageHeight);
    SDL_Log("Number of Tiles: %zu\n", mTiles.size());
    for (const auto& tile : mTiles) {
        SDL_Log("  Tile ID: %d\n", tile.id);
        if (!tile.mObjectGroup.is_null()) {
            SDL_Log("    Object Group: %s\n", tile.mObjectGroup.dump().c_str());
        }
    }
}