#pragma once

#include <string>
#include <map>
#include <string>
#include "../libs/Json.h"
#include "./Tileset.h"
#include "../actors/MapObject.h"
#include "../actors/Tile.h"

class Game;

using json = nlohmann::json;

class Map
{
public:
    Map(class Game* game, std::string jsonPath);
    ~Map();

    void Print();
    int GetWidth() const { return mWidth; }
    int GetHeight() const { return mHeight; }

private:
    class Game* mGame;
    int mWidthInTiles, mHeightInTiles;
    int mWidth, mHeight;
    std::vector<class Tile*> mTiles;
    std::vector<class MapObject*> mMapObjects;
    std::map<std::string, class Tileset> mTilesets;

    std::map<std::string, class Tileset> LoadAllAvailableTilesets(const std::string& baseTilesetsPath);
    std::vector<std::pair<std::string, int>> LoadTilsetsUsedInMap(const json &data, const std::string &baseTilesetsPath, std::map<std::string, Tileset> &allAvailableTilesets);
    void LoadObjectsLayer(const json& layerData, int layerIdx);
    void LoadTilesLayer(std::vector<std::pair<std::string, int>> &nameToFirstGID, const json &layerData, int layerIdx);
};
