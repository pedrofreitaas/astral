#include "./Tileset.h"
#include <fstream>

Tileset::Tileset(Game *game, std::string jsonPath)
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

    for (auto &element : data["tiles"].items())
    {
        int id = element.value()["id"];
        int BBOffsetX = 0;
        int BBOffsetY = 0;
        int BBWidth = 0;
        int BBHeight = 0;
        bool hasCollision = true;

        if (
            element.value().contains("objectgroup") &&
            element.value()["objectgroup"].contains("objects") && 
            element.value()["objectgroup"]["objects"].size() > 0)
        {
            const json &obj = element.value()["objectgroup"]["objects"][0];
            BBOffsetX = obj["x"];
            BBOffsetY = obj["y"];
            BBWidth = obj["width"];
            BBHeight = obj["height"];
        }

        if (element.value().contains("properties")) {
            for (const auto &prop : element.value()["properties"])
            {
                std::string propName = prop["name"].get<std::string>();
                const json &propValue = prop["value"];

                if (propName == "hasCollision")
                {
                    if (!propValue.get<bool>())
                    {
                        hasCollision = false;
                    }
                }
            }
        }
            
        TileExtraInfo extInfo(id, BBOffsetX, BBOffsetY, BBWidth, BBHeight, hasCollision);

        mTileExtraInfo.emplace(extInfo.id, extInfo);
    }
}

Tileset::~Tileset()
{
    if (!mTexture)
    {
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

void Tileset::LoadTexture()
{
    const std::string basePath = "../assets/Levels/Tilesets/Textures/";

    const std::string texturePath = basePath + mName + ".png";

    SDL_Texture *texture = mGame->LoadTexture(texturePath);

    SDL_Log("Loading tileset texture: %s", texturePath.c_str());

    if (!texture)
    {
        throw std::runtime_error("Failed to load tileset texture: " + texturePath);
    }

    mTexture = texture;
}

Vector2 Tileset::GetTilesetTexturePosition(int localGID)
{
    int tilesPerRow = mImageWidth / mTileWidth;
    int x = (localGID % tilesPerRow) * mTileWidth;
    int y = (localGID / tilesPerRow) * mTileHeight;

    return Vector2(x, y);
}

Vector2 Tileset::GetBBOffset(int localID)
{
    const auto &extInfo = mTileExtraInfo.find(localID);

    if (extInfo != mTileExtraInfo.end())
    {
        return Vector2(
            extInfo->second.BBOffsetX,
            extInfo->second.BBOffsetY);
    }

    return Vector2(0.0f, 0.0f);
}

Vector2 Tileset::GetBBSize(int localID)
{
    const auto &extInfo = mTileExtraInfo.find(localID);

    if (extInfo != mTileExtraInfo.end())
    {
        return Vector2(
            extInfo->second.BBWidth,
            extInfo->second.BBHeight);
    }

    if (extInfo->second.hasCollision)
        return Vector2(mTileWidth, mTileHeight);

    return Vector2(0.0f, 0.0f);
}