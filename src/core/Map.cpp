#include <SDL.h>
#include <fstream>
#include "./Map.h"

std::map<std::string, Tileset> Map::LoadAllAvailableTilesets(const std::string &baseTilesetsPath)
{
	std::map<std::string, Tileset> allAvailableTilesets;

	// populate with all tilesets in the base path - this can get costy if there are many tilesets
	for (const auto &entry : std::filesystem::directory_iterator(baseTilesetsPath))
	{
		if (entry.path().extension() != ".json")
		{
			SDL_Log("Skipping non-json file: %s", entry.path().string().c_str());
			continue; // <-- important
		}

		std::string tilesetPath = entry.path().string();
		Tileset t(tilesetPath);

		if (t.GetName().empty())
		{
			SDL_Log("Skipping tileset with empty name: %s", tilesetPath.c_str());
			continue;
		}

		if (allAvailableTilesets.find(t.GetName()) != allAvailableTilesets.end())
		{
			SDL_Log("Warning: Duplicate tileset name found: %s. Overwriting previous entry.", t.GetName().c_str());
		}

		allAvailableTilesets[t.GetName()] = t;
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

	for (const auto &tilesetData : data["tilesets"])
	{
		std::string tilesetName = tilesetData["source"];

		tilesetName = tilesetName.substr(0, tilesetName.find_last_of('.'));

		Tileset t = allAvailableTilesets[tilesetName];

		if (t.GetName().empty())
		{
			SDL_Log("Warning: Tileset %s not found in available tilesets. Skipping.", tilesetName.c_str());
			continue;
		}

		mTilesets[tilesetData["name"]] = t;
	}

	mLayers = std::vector<Layer>();

	for (const auto &layerData : data["layers"])
	{
		Layer layer;
		layer.pos = layerData["id"];
		layer.width = layerData["width"];
		layer.height = layerData["height"];
		layer.tileIds = layerData["data"].get<std::vector<int>>();

		mLayers.push_back(layer);
	}
}

Map::~Map() {}

void Map::print()
{
	SDL_Log("Map class loaded.");
	SDL_Log("Map Size: %dx%d", mapWidth, mapHeight);
	SDL_Log("Tile Size: %dx%d", tileWidth, tileHeight);
	SDL_Log("Number of Tilesets: %zu", mTilesets.size());
	SDL_Log("Number of Layers: %zu", mLayers.size());
}