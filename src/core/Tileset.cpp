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
        TileExtraInfo extInfo;
        
        extInfo.id = element.value()["id"];
        
        if (element.value().contains("objectgroup")) {
            extInfo.mObjectGroup = element.value()["objectgroup"];
        }
        
        mTileExtraInfo.insert_or_assign(extInfo.id, std::move(extInfo));
    }
}

Tileset::~Tileset() {
    if (!mTexture) {
        return;
    }

    SDL_Log("TO-DO look on how to delete texture pointer");
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

    if (!texture) {
        SDL_Log("Failed to load tileset texture: %s", texturePath.c_str());
        return;
    }

    mTexture = texture;
}