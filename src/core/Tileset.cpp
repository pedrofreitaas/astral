#include "./Tileset.h"
#include <fstream>

Tileset::Tileset(Game* game, std::string jsonPath) 
: mGame(game)
{
    std::ifstream file(jsonPath);
    json data = json::parse(file);

    mTileHeight = data["tileheight"];
    mTileWidth = data["tilewidth"];
    mImageHeight = data["imageheight"];
    mImageWidth = data["imagewidth"];
    mName = data["name"];

    LoadTexture();

    mTileExtraInfo = std::map<int, TileExtraInfo>();

    for (auto& element : data["tiles"].items()) {
        if (!element.value().contains("objectgroup")) {
            throw std::runtime_error("Tileset JSON missing objectgroup for tile id.");
        }

        int id = element.value()["id"];
        int BBOffsetX = element.value()["objectgroup"]["objects"][0]["x"];
        int BBOffsetY = element.value()["objectgroup"]["objects"][0]["y"];
        int BBWidth = element.value()["objectgroup"]["objects"][0]["width"];
        int BBHeight = element.value()["objectgroup"]["objects"][0]["height"];

        TileExtraInfo extInfo(id, BBOffsetX, BBOffsetY, BBWidth, BBHeight);
        
        mTileExtraInfo.emplace(extInfo.id, extInfo);
    }
}

Tileset::~Tileset() {
    if (!mTexture) {
        return;
    }
}

void Tileset::Print()
{
    SDL_Log("Name: %s\n", mName.c_str());
    SDL_Log("Tile Size: %dx%d\n", mTileWidth, mTileHeight);
    SDL_Log("Image Size: %dx%d\n", mImageWidth, mImageHeight);
    SDL_Log("Number of Extra infos for tiles: %zu\n", mTileExtraInfo.size());
}

void Tileset::LoadTexture() {
    const std::string basePath = "../assets/Levels/Tiles/";

    const std::string texturePath = basePath + mName + ".png";

    SDL_Texture* texture = mGame->LoadTexture(texturePath);

    SDL_Log("Loading tileset texture: %s", texturePath.c_str());

    if (!texture) {
        throw std::runtime_error("Failed to load tileset texture: " + texturePath);
    }

    mTexture = texture;
}

Vector2 Tileset::GetGridDims(int localGID) {
    int tilesPerRow = mImageWidth / mTileWidth;
    int x = (localGID % tilesPerRow) * mTileWidth;
    int y = (localGID / tilesPerRow) * mTileHeight;
    return Vector2(x, y);
}