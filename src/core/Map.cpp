#include <SDL.h>
#include <fstream>
#include "./Map.h"
#include "./Game.h"

std::map<std::string, Tileset> Map::LoadAllAvailableTilesets(const std::string &baseTilesetsPath)
{
	std::map<std::string, Tileset> allAvailableTilesets;

	// populate with all tilesets in the base path - this can get costy if there are many tilesets
	for (const auto &entry : std::filesystem::directory_iterator(baseTilesetsPath))
	{
		if (entry.path().extension() != ".json")
		{
			SDL_Log("Skipping non-json file: %s", entry.path().string().c_str());
			continue;
		}

		std::string tilesetPath = entry.path().string();
		Tileset t(mGame, tilesetPath);

		if (t.GetName().empty())
		{
			SDL_Log("Skipping tileset with empty name: %s", tilesetPath.c_str());
			continue;
		}

		if (allAvailableTilesets.find(t.GetName()) != allAvailableTilesets.end())
		{
			SDL_Log("Warning: Duplicate tileset name found: %s. Overwriting previous entry.", t.GetName().c_str());
		}

		// this line is a major issue
		// allAvailableTilesets[t.GetName()] = t;
		allAvailableTilesets.insert_or_assign(t.GetName(), std::move(t));
	}

	// peformance warning if too many tilesets
	if (allAvailableTilesets.size() > 15)
	{
		SDL_Log("Warning: More than 15 tilesets found (%zu). This may impact performance.", allAvailableTilesets.size());
	}

	return allAvailableTilesets;
}

Map::Map(Game *game, std::string jsonPath)
{
	const std::string basePath = "../assets/Levels/Maps/";
	const std::string baseTilesetsPath = "../assets/Levels/Tilesets/";

	jsonPath = basePath + jsonPath;
	std::ifstream file(jsonPath);
	json data = json::parse(file);

	mGame = game;

	tileHeight = data["tileheight"];
	tileWidth = data["tilewidth"];
	mapHeight = data["height"];
	mapWidth = data["width"];

	mTilesets = std::map<std::string, Tileset>();

	std::map<std::string, Tileset> allAvailableTilesets = LoadAllAvailableTilesets(baseTilesetsPath);
	std::map<std::string, int> nameToFirstGID;

	// Load tilesets used in this map
	for (const auto &tilesetData : data["tilesets"])
	{
		std::string tilesetName = tilesetData["source"];
		int firstGID = tilesetData["firstgid"];

		tilesetName = tilesetName.substr(0, tilesetName.find_last_of('.'));

		// Tileset t = allAvailableTilesets[tilesetName]; - this line is a major issue
		auto it = allAvailableTilesets.find(tilesetName);
		if (it == allAvailableTilesets.end())
		{
			SDL_Log("Warning: Tileset %s not found in available tilesets. Skipping.", tilesetName.c_str());
			throw std::runtime_error("Tileset not found: " + tilesetName);
		}
		Tileset t = it->second;

		if (t.GetName().empty())
		{
			SDL_Log("Warning: Tileset %s not found in available tilesets. Skipping.", tilesetName.c_str());
			continue;
		}

		mTilesets.emplace(tilesetName, t);
		nameToFirstGID.insert_or_assign(tilesetName, std::move(firstGID));
	}

	mTiles = std::vector<class Tile*>();

	// Iterate through each layer and extract tiles
	SDL_Log("TO-DO: draw order for tiles implementation");
	for (const auto &layerData : data["layers"])
	{
		for (const auto &tileGID : layerData["data"])
		{
			int gid = tileGID.get<int>();

			if (gid == 0)
				continue;

			std::string tilesetName = "";
			int firstGID = 0;

			for (const auto &pair : nameToFirstGID)
			{
				if (gid >= pair.second)
				{
					tilesetName = pair.first;
					firstGID = pair.second;
				}
			}

			const auto &search = mTilesets.find(tilesetName);

			if (search == mTilesets.end())
			{
				throw std::runtime_error("Tile extraction: Tileset not found in map tilesets: " + tilesetName);
			}

			Tileset currentTileset = search->second;

			int localID = gid - firstGID - 1; // -1 because Tiled starts at 1

			SDL_Texture *texture = currentTileset.GetTexture();
			Vector2 grid = currentTileset.GetGridDims(localID);
			Vector2 tileDims = currentTileset.GetTileDims();
			Vector2 bbOffset = currentTileset.GetBBOffset(localID);
			Vector2 bbSize = currentTileset.GetBBSize(localID);

			mTiles.push_back(
				std::move(
					new Tile(
						mGame,
						texture,
						grid.x, grid.y,			// gridX and Y
						tileDims.x, tileDims.y, // height and width
						bbSize.x, bbSize.y,		// boundBox size
						bbOffset.x, bbOffset.y, // boundBox
						DrawLayerPosition::Ground
					)
				)
			);
		}
	}
}

Map::~Map()
{
	mTiles.clear();
	mTilesets.clear();
}

void Map::Print()
{
	SDL_Log("Map class loaded.");
	SDL_Log("Map Size: %dx%d", mapWidth, mapHeight);
	SDL_Log("Tile Size: %dx%d", tileWidth, tileHeight);
	SDL_Log("Number of Tilesets: %zu", mTilesets.size());
	for (const auto &pair : mTilesets)
	{
		SDL_Log("Tileset: %s", pair.first.c_str());
	}
	SDL_Log("Number of Layers: %zu", mTiles.size());
}