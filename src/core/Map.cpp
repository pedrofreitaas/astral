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

void Map::LoadTilesLayer(std::vector<std::pair<std::string, int>> &nameToFirstGID, const json &layerData, int layerIdx)
{
	int tileIdx = -1;
	for (const auto &tileGID : layerData["data"])
	{
		tileIdx++;
		int gid = tileGID.get<int>();

		if (gid == 0)
			continue;

		std::string tilesetName = "";
		int firstGID = 0;

		for (std::pair<std::string, int> &pair : nameToFirstGID)
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

		int localID = gid - firstGID;

		SDL_Texture *texture = currentTileset.GetTexture();
		Vector2 tileDims = currentTileset.GetTileDims();
		Vector2 worldPosition(
			tileDims.x * (tileIdx % mWidthInTiles),
			tileDims.y * std::floor(tileIdx * 1.0f / mWidthInTiles));
		Vector2 tilesetPosition = currentTileset.GetTilesetTexturePosition(localID);
		Vector2 bbOffset = currentTileset.GetBBOffset(localID);
		Vector2 bbSize = currentTileset.GetBBSize(localID);

		DrawLayerPosition layer =
			(layerIdx == 0)	  ? static_cast<DrawLayerPosition>(static_cast<int>(DrawLayerPosition::Player) - 10)
			: (layerIdx == 1) ? DrawLayerPosition::Player
							  : static_cast<DrawLayerPosition>(static_cast<int>(DrawLayerPosition::Player) + 10);

		mTiles.push_back(
			std::move(
				new Tile(
					mGame,
					texture,
					worldPosition,
					tilesetPosition,
					tileDims.x, tileDims.y,
					bbSize.x, bbSize.y,		// boundBox size
					bbOffset.x, bbOffset.y, // boundBox
					layer)));
	}
}

void Map::LoadObjectsLayer(const json &layerData, int layerIdx)
{
	int objectIdx = -1;
	for (const auto &obj : layerData["objects"])
	{
		objectIdx++;
		int id = obj["id"].get<int>();
		int x = obj["x"].get<int>();
		int y = obj["y"].get<int>();
		int width = obj["width"].get<int>();
		int height = obj["height"].get<int>();
		std::string ev, function_name;
		json parameters = json::object();

		for (const auto &prop : obj["properties"])
		{
			std::string propName = prop["name"].get<std::string>();
			std::string propValue = prop["value"].get<std::string>(); 
			
			if (propName == "event")
				ev = propValue;
			else if (propName == "function_name")
				function_name = propValue;
			// every other property goes into parameters json
			else
				parameters[propName] = propValue;
		}

		MapObject *mapObject = new MapObject(
			mGame,
			id,
			ev,
			function_name,
			Vector2(x, y),
			Vector2(width, height),
			parameters);

		mMapObjects.push_back(mapObject);
	}
}

std::vector<std::pair<std::string, int>> Map::LoadTilsetsUsedInMap(const json &data, const std::string &baseTilesetsPath, std::map<std::string, Tileset> &allAvailableTilesets)
{
	std::vector<std::pair<std::string, int>> nameToFirstGID;

	// Load tilesets used in this map
	for (const auto &tilesetData : data["tilesets"])
	{
		std::string tilesetName = tilesetData["source"];
		int firstGID = tilesetData["firstgid"];

		// Remove directory path and extension from tilesetName
		size_t lastSlash = tilesetName.find_last_of("/\\");
		if (lastSlash != std::string::npos)
		{
			tilesetName = tilesetName.substr(lastSlash + 1);
		}
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

		SDL_Log("Loaded tileset: %s with firstGID: %d", tilesetName.c_str(), firstGID);

		mTilesets.emplace(tilesetName, t);
		nameToFirstGID.push_back(std::pair<std::string, int>(tilesetName, firstGID));
	}

	return nameToFirstGID;
}

Map::Map(Game *game, std::string jsonPath)
{
	const std::string basePath = "../assets/Levels/Maps/";
	const std::string baseTilesetsPath = "../assets/Levels/Tilesets/";

	jsonPath = basePath + jsonPath;
	std::ifstream file(jsonPath);
	json data = json::parse(file);

	mGame = game;
	mHeightInTiles = data["height"];
	mWidthInTiles = data["width"];
	int tileWidth = data["tilewidth"];
	int tileHeight = data["tileheight"];
	mWidth = mWidthInTiles * tileWidth;
	mHeight = mHeightInTiles * tileHeight;

	mTilesets = std::map<std::string, Tileset>();
	std::map<std::string, Tileset> allAvailableTilesets = LoadAllAvailableTilesets(baseTilesetsPath);
	std::vector<std::pair<std::string, int>> nameToFirstGID = LoadTilsetsUsedInMap(
		data,
		baseTilesetsPath,
		allAvailableTilesets);

	mTiles = std::vector<class Tile *>();

	int layerIdx = -1;
	for (const auto &layerData : data["layers"])
	{
		layerIdx++;
		int tileIdx = -1;

		if (layerData["name"] == "objects")
		{
			LoadObjectsLayer(layerData, layerIdx);
			continue;
		}

		LoadTilesLayer(nameToFirstGID, layerData, layerIdx);
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
	SDL_Log("Map Size: %dx%d", mWidthInTiles, mHeightInTiles);
	SDL_Log("Number of Tilesets: %zu", mTilesets.size());
	for (const auto &pair : mTilesets)
	{
		SDL_Log("Tileset: %s", pair.first.c_str());
	}
	SDL_Log("Number of Layers: %zu", mTiles.size());
}