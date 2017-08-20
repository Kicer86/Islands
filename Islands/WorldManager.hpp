#pragma once

#include <SFML/System/Clock.hpp>
#include <noise/noise.h>

#include <ctime>
#include <memory>

#include "ErrorHandler.hpp"
#include "DefContainer.hpp"
#include "Object.hpp"
#include "World.hpp"

class WorldManager
{
	std::shared_ptr<World> ManagementWorld;
	std::shared_ptr<ItemDefContainer> ItemsDef;
	std::shared_ptr<ObjectDefContainer> ObjectsDef;
	std::shared_ptr<sf::Clock> GameClockPtr;
	std::vector<Structure> Structures;

public:
	void AssingStructures(std::vector<Structure> &Structs)
	{
		this->Structures = Structs;
	}
	void AssingItemsDef(std::shared_ptr<ItemDefContainer> &Items)
	{
		ItemsDef = Items;
	}
	void AssingObjectsDef(std::shared_ptr<ObjectDefContainer> &ObjectDef)
	{
		this->ObjectsDef = ObjectDef;
	}
	void AssingWorld(std::shared_ptr<World> &World)
	{
		this->ManagementWorld = World;
	}
	void AssingClock(sf::Clock &Clock)
	{
		GameClockPtr = std::make_shared<sf::Clock>(Clock);
	}


	void buildLocalMap(TerrainType Base,size_t LocalMapSize = World::DefaultMapSize)
	{
		sf::Clock TestClock;

		noise::module::Perlin noiseModule;
		noiseModule.SetSeed(static_cast<int>(time(time_t(NULL))));
		noiseModule.SetOctaveCount(6);
		noiseModule.SetFrequency(1.0);
		noiseModule.SetPersistence(0.25);

		ManagementWorld->resizeLocalMap(LocalMapSize);

		double noise;
		TerrainType Terrain = TerrainType::Null;

		for (size_t x = 0; x < ManagementWorld->getLocalMapSize(); x++)
		{
			for (size_t y = 0; y < ManagementWorld->getLocalMapSize(); y++)
			{
				noise = noiseModule.GetValue(1.25 + (0.1 * x), 0.75 + (0.1 * y), 0.5);
				if (noise > 0.75)
				{
					Terrain = TerrainType::Rock;
				}
				else if (noise > 0.35)
				{
					Terrain = TerrainType::Dirt;
				}
				else if (noise > -0.25)
				{
					Terrain = TerrainType::Grass;
				}
				else
				{
					Terrain = TerrainType::Water;
				}
				ManagementWorld->setLocalMapTileTerrain(sf::Vector2u(x, y), Terrain);
			}
		}

		ErrorHandler::logToFile("Generate Local Map [Size] = " + std::to_string(ManagementWorld->getLocalMapSize()) +
			" [Time] = " + std::to_string(TestClock.getElapsedTime().asMilliseconds()) + " milisecs ");
	}
	void buildWorld(size_t WorldSize = World::DefaultWorldSize,unsigned Seed = 1)
	{

	}


	bool placeObject(sf::Vector2u tile, unsigned ObjectId)
	{
		if (ObjectId >= ObjectsDef->getSize() || ManagementWorld->getLocalMapTileObject(tile) != nullptr)
		{
			return false;
		}
		if (!sf::Rect<unsigned>(sf::Vector2u(),sf::Vector2u(ManagementWorld->getLocalMapSize(),
			ManagementWorld->getLocalMapSize())).contains(tile))
		{
			return false;
		}
		

		switch (ObjectsDef->getDefinition(ObjectId)->getType())
		{
		case ObjectType::Default:
			ManagementWorld->setLocalMapTileObject(tile, new Object(ObjectId));
			return true;
			break;
		case ObjectType::Chest:
			ManagementWorld->setLocalMapTileObject(tile, new ChestObject(ObjectId,
				dynamic_cast<ChestDef*>(ObjectsDef->getDefinition(ObjectId))->getCapacity()));
			return true;
			break;
		case ObjectType::CraftingPlace:
			ManagementWorld->setLocalMapTileObject(tile, new CraftingPlaceObject(ObjectId, makeFromDef::makeRecipe(dynamic_cast<CraftingPlaceDef*>(ObjectsDef->getDefinition(ObjectId))->getRecipes(),*ItemsDef.get())));
			return true;
			break;
		case ObjectType::Tree:
			break;
		case ObjectType::Sapling:
			ManagementWorld->setLocalMapTileObject(tile, new SaplingObject(ObjectId, GameClockPtr->getElapsedTime().asSeconds()));
			return true;
			break;
		case ObjectType::Spawner:
			break;
		default:
			break;
		}
		return false;
	}
	bool placeStructure(sf::Vector2u begin,unsigned StructureId)
	{
		if (StructureId >= Structures.size())
		{
			return false;
		}
		if (!sf::Rect<unsigned>(sf::Vector2u(), sf::Vector2u(ManagementWorld->getLocalMapSize(),
			ManagementWorld->getLocalMapSize())).contains(begin))
		{
			return false;
		}


		return true;
	}


	sf::IntRect getLocalMapTileCollisionBox(sf::Vector2u tile)
	{
		if (!sf::Rect<unsigned>(sf::Vector2u(), sf::Vector2u(ManagementWorld->getLocalMapSize(),
			ManagementWorld->getLocalMapSize())).contains(tile))
		{
			return sf::IntRect();
		}
		if (ManagementWorld->getLocalMapTileObject(tile) == nullptr)
		{
			return sf::IntRect();
		}


		sf::IntRect collideObject;
		unsigned collideObjectId = ManagementWorld->getLocalMapTileObject(tile)->Id;
		if (collideObjectId != 0)
		{
			sf::FloatRect collideObjectBox = ObjectsDef->getDefinition(collideObjectId)->getCollisionBox();
			if (collideObjectBox.top <= 0.01f && collideObjectBox.left <= 0.01f
				&& collideObjectBox.height <= 0.01f && collideObjectBox.width <= 0.01f)
			{
				return collideObject;
			}
			collideObject.top = tile.y * static_cast<int>(TILE_SIZE);
			collideObject.left = tile.x * static_cast<int>(TILE_SIZE);
			collideObject.width = static_cast<int>(TILE_SIZE);
			collideObject.height = static_cast<int>(TILE_SIZE);

			if (collideObjectBox.top < 1.0f)
			{
				collideObject.top += static_cast<int>(TILE_SIZE * collideObjectBox.top);
			}
			if (collideObjectBox.left < 1.0f)
			{
				collideObject.left += static_cast<int>(TILE_SIZE * collideObjectBox.left);
			}
			if (collideObjectBox.height < 1.0f)
			{
				collideObject.height -= static_cast<int>(TILE_SIZE * collideObjectBox.height);
			}
			if (collideObjectBox.width < 1.0f)
			{
				collideObject.width -= static_cast<int>(TILE_SIZE * collideObjectBox.width);
			}
		}
		return collideObject;
	}
};