#include "Engine.hpp"

void Engine::loadGameComponents()
{
	GameComponentsLoader loader;
	std::string graphicsfile;
	RawObjects.generateArray(loader.LoadObjectDefFromFile(graphicsfile));

	for (auto & i : RawObjects.getObjects())
	{
		auto ref = const_cast<ObjectDef&>(i);
			mediaContainer.pushTexture(TextureContainer::ObjectTextures,
				graphicsfile, ref.getTextureCord());
			mediaContainer.pushTexture(TextureContainer::ItemsTextures,
				graphicsfile, ref.getTextureCord());
	}
	loader.GenerateItemsFromObjectDef(RawObjects.getObjects(), Items);

	for (auto & i : Items.getContainer())
	{
			ErrorHandler::log("Load Item: "+i->getName()+" maxStack: 64");
	}

	std::string itemTextureFile;
	std::vector<sf::IntRect> ItemTextRect;
	loader.LoadItemDefFromFile(Items.getContainer(), itemTextureFile, ItemTextRect);

	for (auto & i : ItemTextRect)
	{
		mediaContainer.pushTexture(TextureContainer::ItemsTextures, itemTextureFile, i);
	}
}

void Engine::checkPlayerEnvironment()
{
	sf::Vector2f playerCollectRectSize(128, 128);
	sf::Vector2f playerCollectRectPos = player.getPosition() - sf::Vector2f(playerCollectRectSize.x / 2,
		playerCollectRectSize.y / 2);

	for (size_t i = 0; i < LyingItems.getSize(); i++)
	{
		if (CollisionDetect::isPointInRectangle(LyingItems.getPosition(i),
			playerCollectRectPos, playerCollectRectSize))
		{
			ItemField temp = LyingItems.getItem(i);
			pushItemToPlayerInventory(temp);
			if (temp.isClear())
			{
				LyingItems.eraseItem(i);
			}
			else
			{
				LyingItems.setItemAmount(i, temp.ItemAmount);
			}
		}
	}
}

void Engine::pushItemToPlayerInventory(ItemField & item)
{
	for (size_t i = 0; i < PlayerFieldsNumber; i++)
	{
		if (player.getHandInventoryField(i).ItemId == item.ItemId)
		{
			ItemField temp = player.getHandInventoryField(i);
			temp += item.ItemAmount;
			unsigned maxStack = Items.getItemDef(item.ItemId)->getMaxStack();

			if (temp.ItemAmount >= maxStack)
			{
				item.ItemAmount = temp.ItemAmount - maxStack;
				temp.ItemAmount -= (temp.ItemAmount - maxStack);
				player.setHandInventoryField(i, temp);
			}
		}
		else if (player.getHandInventoryField(i).ItemId == 0)
		{
			player.setHandInventoryField(i, item);	
			item.clear();
			return;
		}
		for (size_t j = 0; j < PlayerFieldsNumber; j++)
		{
			if (player.getInventoryField(sf::Vector2u(i,j)).ItemId == item.ItemId)
			{
				ItemField temp = player.getInventoryField(sf::Vector2u(i, j));
				temp += item.ItemAmount;
				unsigned maxStack = Items.getItemDef(item.ItemId)->getMaxStack();
				if (temp.ItemAmount >= maxStack)
				{
					item.ItemAmount = temp.ItemAmount - maxStack;
					temp.ItemAmount -= (temp.ItemAmount - maxStack);
					player.setInventoryField(sf::Vector2u(i,j), temp);
				}
			}
			else if (player.getInventoryField(sf::Vector2u(i,j)).ItemId == 0)
			{
				player.setInventoryField(sf::Vector2u(i, j), item);
				item.clear();
				return;
			}
		}
	}
}

void Engine::checkPlayerBehaviour(IslandApp &app)
{
	sf::Vector2f movevctr;
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::A) || sf::Keyboard::isKeyPressed(sf::Keyboard::Left))
	{
		movevctr.x -= 5;
	}
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::D) || sf::Keyboard::isKeyPressed(sf::Keyboard::Right))
	{
		movevctr.x += 5;
	}
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::W) || sf::Keyboard::isKeyPressed(sf::Keyboard::Up))
	{
		movevctr.y -= 5;
	}
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::S) || sf::Keyboard::isKeyPressed(sf::Keyboard::Down))
	{
		movevctr.y += 5;
	}

	player.move(movevctr);
}

void Engine::spawnPlayer()
{
	const int MaxPosition = Map::MAP_SIZE - 10;

	srand(static_cast<unsigned int>(time(NULL)));
	sf::Vector2f spawnPoint;

	while (true)
	{
		spawnPoint = sf::Vector2f(static_cast<float>(rand() % MaxPosition * 64),
			static_cast<float>(rand() % MaxPosition * 64));

		if (GameWorld.isPlaceImpassable(sf::Vector2f(spawnPoint.y,spawnPoint.x)) == false)
		{
			ErrorHandler::log(std::string("Spawn player position:"));
			ErrorHandler::log("Tile Y " + std::to_string(Map::getTiledPosition(spawnPoint).y));
			ErrorHandler::log("Tile X " + std::to_string(Map::getTiledPosition(spawnPoint).x));
			break;
		}
	}
	player.setPosition(spawnPoint);
	player.setSpawnPoint(spawnPoint);
}

void Engine::pushChangesToGui()
{
	for (size_t i = 0; i < PlayerFieldsNumber; i++)
	{
		if (i < 3)
		{
			GameGui.EquipmentGui.clearArmorField(i);
			if (GameGui.EquipmentGui.getHoverFromArmorField(i))
			{
				GameGui.EquipmentGui.getArmorFieldRect(i).setFillColor(EqFieldColorWhenIsSelected);
			}

			unsigned ArmorItemId = player.getArmorInventoryField(i).ItemId;
			if (ArmorItemId != 0)
			{
				GameGui.EquipmentGui.pushTextureToArmorFields(i, &mediaContainer.getTexture(ArmorItemId,
					TextureContainer::ItemsTextures));
			}
		}

		GameGui.HudGui.clearBeltField(i);
		if (i == GameGui.getNumberOfSelectedBeltField())
		{
			GameGui.HudGui.getBeltFieldRect(i).setFillColor(EqFieldColorWhenIsSelected);
		}
		if (GameGui.HudGui.getHoverFromBeltField(i))
		{
			GameGui.HudGui.getBeltFieldRect(i).setFillColor(EqFieldColorWhenIsSelected);
		}

		unsigned itemId = player.getHandInventoryField(i).ItemId;

		if (itemId != 0)
		{
			GameGui.HudGui.pushTextureToBeltField(i, &mediaContainer.getTexture(itemId,
				TextureContainer::ItemsTextures));
		}

		for (size_t j = 0; j < PlayerFieldsNumber; j++)
		{
			GameGui.EquipmentGui.clearInventoryField(sf::Vector2u(i, j));
			if (GameGui.EquipmentGui.getHoverFromInventoryField(sf::Vector2u(i,j)))
			{
				GameGui.EquipmentGui.getFieldRect(sf::Vector2u(i, j)).setFillColor(EqFieldColorWhenIsSelected);
			}

			unsigned tempItemId = player.getInventoryField(sf::Vector2u(i, j)).ItemId;
			if (tempItemId != 0)
			{
				GameGui.EquipmentGui.pushTextureToFields(sf::Vector2u(i, j), &mediaContainer.getTexture(tempItemId,
					TextureContainer::ItemsTextures));
			}
		}

	}
}

void Engine::checkGuiOperations(EquipmentType type, sf::Vector2u field)
{
	unsigned holdedItemId = GameGui.getHoldedItem().ItemId;
	switch (type)
	{
	case EquipmentType::Inventory:
		if (holdedItemId != 0)
		{
			if (player.getInventoryField(field).ItemId == holdedItemId)
			{
				player.setInventoryField(field, ItemField(holdedItemId,
					player.getInventoryField(field).ItemAmount + GameGui.getHoldedItem().ItemAmount));

				unsigned maxStack = Items.getItemDef(holdedItemId)->getMaxStack();
				if (player.getInventoryField(field).ItemAmount >= maxStack)
				{
					unsigned amountForHold = player.getInventoryField(field).ItemAmount - maxStack;
					player.setInventoryField(field, ItemField(holdedItemId,
						player.getInventoryField(field).ItemAmount - amountForHold));

					GameGui.assingNewItemToHold(ItemField(holdedItemId, amountForHold));
				}
				else
				{
					GameGui.clearHoldedItem();
				}

			}
			else if(player.getInventoryField(field).ItemId != 0)
			{
				ItemField temp(player.getInventoryField(field));
				player.setInventoryField(field, GameGui.getHoldedItem());
				GameGui.assingNewItemToHold(temp);
			}
			else
			{
				player.setInventoryField(field, GameGui.getHoldedItem());
				GameGui.clearHoldedItem();
			}
		}
		else
		{
			if (player.getInventoryField(field).ItemId != 0)
			{
				GameGui.assingNewItemToHold(player.getInventoryField(field));
				player.setInventoryField(field, ItemField());
			}
		}
		break;
	case EquipmentType::Armor:
		if (holdedItemId != 0)
		{
			if (player.getArmorInventoryField(field.x).ItemId != 0)
			{
				ItemField temp(player.getArmorInventoryField(field.x));
				player.setArmorField(field.x, GameGui.getHoldedItem());
				GameGui.assingNewItemToHold(temp);
			}
			else
			{
				player.setArmorField(field.x, GameGui.getHoldedItem());
				GameGui.clearHoldedItem();
			}
		}
		else
		{
			if (player.getArmorInventoryField(field.x).ItemId != 0)
			{
				GameGui.assingNewItemToHold(player.getArmorInventoryField(field.x));
				player.setArmorField(field.x, ItemField());
			}
		}
		break;
	case EquipmentType::Belt:
		if (holdedItemId != 0)
		{
			if (player.getHandInventoryField(field.x).ItemId == holdedItemId)
			{
				player.setHandInventoryField(field.x, ItemField(holdedItemId,
					player.getHandInventoryField(field.x).ItemAmount + GameGui.getHoldedItem().ItemAmount));

				unsigned maxStack = Items.getItemDef(holdedItemId)->getMaxStack();
				if (player.getHandInventoryField(field.x).ItemAmount >= maxStack)
				{
					unsigned amountForHold = player.getHandInventoryField(field.x).ItemAmount - maxStack;
					player.setHandInventoryField(field.x, ItemField(holdedItemId,
						player.getHandInventoryField(field.x).ItemAmount - amountForHold));

					GameGui.assingNewItemToHold(ItemField(holdedItemId, amountForHold));
				}
				else
				{
					GameGui.clearHoldedItem();
				}

			}
			else if (player.getHandInventoryField(field.x).ItemId != 0)
			{
				ItemField temp(player.getHandInventoryField(field.x));
				player.setHandInventoryField(field.x, GameGui.getHoldedItem());
				GameGui.assingNewItemToHold(temp);
			}
			else
			{
				player.setHandInventoryField(field.x, GameGui.getHoldedItem());
				GameGui.clearHoldedItem();
			}
		}
		else
		{
			if (player.getHandInventoryField(field.x).ItemId != 0)
			{
				GameGui.assingNewItemToHold(player.getHandInventoryField(field.x));
				player.setHandInventoryField(field.x, ItemField());
			}
		}
		break;
	default:
		break;
	}
}

void Engine::drawTile(sf::Vector2u tileIndex, sf::RenderWindow & window,sf::RectangleShape &shp)
{
	TILE tile = GameWorld.getTile(sf::Vector2u(tileIndex.y,tileIndex.x));
	if (tile == TILE::EMPTY) { return; }
	shp.setPosition(sf::Vector2f(Map::getNormalPosition(sf::Vector2i(tileIndex.x,tileIndex.y))));
	switch (tile)
	{
	case TILE::EMPTY:
		break;
	case TILE::DIRT:
		shp.setTexture(&mediaContainer.getTexture(1,TextureContainer::TileTextures));
		break;
	case TILE::GRASS:
		shp.setTexture(&mediaContainer.getTexture(2, TextureContainer::TileTextures));
		break;
	case TILE::ROCK:
		shp.setTexture(&mediaContainer.getTexture(4, TextureContainer::TileTextures));
		break;
	case TILE::BRIGDE:
		shp.setTexture(&mediaContainer.getTexture(2, TextureContainer::TileTextures));
		break;
	case TILE::CLOUD:
		shp.setTexture(&mediaContainer.getTexture(6, TextureContainer::TileTextures));
		break;
	default:
		break;
	}
	window.draw(shp);
}

void Engine::drawObject(sf::Vector2u objectIndex, sf::RenderWindow & window, sf::RectangleShape &shp)
{
	unsigned ObjectID = GameWorld.getObject(sf::Vector2u(objectIndex.y,objectIndex.x));
	if (ObjectID == 0) { return; }
	if (ObjectID > RawObjects.getObjects().size()) { return; }
	shp.setPosition(sf::Vector2f(Map::getNormalPosition(sf::Vector2i(objectIndex.x, objectIndex.y))));
	shp.setTexture(&mediaContainer.getTexture(ObjectID, TextureContainer::ObjectTextures));
	window.draw(shp);
}

bool Engine::checkPlayerPos()
{
	sf::Vector2f playerPos = player.getCharacterCenterPosition();
	if (GameWorld.isPlaceImpassable(sf::Vector2f(playerPos.y,playerPos.x)) == true) { return false; }

	return true;
}

Engine::~Engine()
{
	GameComponentUnloader unloader;
	unloader.clearItems(Items.getContainer());
	ErrorHandler::log("Clear data");
}

void Engine::init()
{
	mediaContainer.load(); ErrorHandler::log("Load media");
	loadGameComponents(); ErrorHandler::log("Load game components");

	player.set(&mediaContainer.getTexture(1,TextureContainer::CharacterTextures), sf::Vector2f(100, 100),sf::Vector2f(10,8));
	camera.setSize(sf::Vector2f(1280, 1024));

	GameWorld.init();
	ErrorHandler::log("Generate map");
	ErrorHandler::log("Map Size " + std::to_string(Map::MAP_SIZE) + " x " + std::to_string(Map::MAP_SIZE));
	spawnPlayer();

	GameGui.create();
}

void Engine::operator()(IslandApp &app,char key,mouseWheel last, bool isMouseClick)
{
	while (true)
	{
		if (LyingItems.getSize() == 0)
		{
			break;
		}
		else if (GameClock.getElapsedTime() - LyingItems.getTime(0) > sf::Time(sf::seconds(600)))
		{
			LyingItems.eraseFirstItem();
		}
		else
		{
			break;
		}
	}

	checkPlayerBehaviour(app);
	checkPlayerEnvironment();

	camera.setCenter(player.getCharacterCenterPosition());

	auto Window = app.getIslandWindow();

	GameGui.setNewPosition(Window->mapPixelToCoords(GameGui.EquipmentGui.defaultEquipmentGuiPosOnScreen));
	GameGui.HudGui.setNewPosition(Window->mapPixelToCoords(GameGui.HudGui.HpInfoScreenPos),
		Window->mapPixelToCoords(GameGui.HudGui.MpInfoScreenPos),Window->mapPixelToCoords(GameGui.HudGui.BeltFieldPosOnScreen));

	if (!checkPlayerPos())
	{ 
		ErrorHandler::log("Player move above map");
		ErrorHandler::log("Pos:X" + std::to_string(player.getCharacterCenterPosition().x) +
			" :Y " + std::to_string(+player.getCharacterCenterPosition().y));
	}

	if (GameGui.getIsEqGuiEnable())
	{
		sf::Vector2f mousePosInWorld = app.getMousePosInWorld();
		for (size_t i = 0; i < PlayerFieldsNumber; i++)
		{
			if (i < 3)
			{
				if (GameGui.EquipmentGui.getHoverFromArmorField(i))
				{
					GameGui.EquipmentGui.setHoverForArmorField(i, false);
				}
				sf::Vector2f fieldPos = GameGui.EquipmentGui.getArmorFieldRect(i).getPosition();

				if (CollisionDetect::isPointInRectangle(mousePosInWorld,fieldPos,
					sf::Vector2f(DefaultEqFieldSize,DefaultEqFieldSize)))
				{
					GameGui.EquipmentGui.setHoverForArmorField(i, true);
					if (isMouseClick)
					{
						checkGuiOperations(EquipmentType::Armor, sf::Vector2u(i, 0));
					}
				}
			}

			if (GameGui.HudGui.getHoverFromBeltField(i))
			{
				GameGui.HudGui.setHoverForBeltField(i, false);
			}
			sf::Vector2f beltFieldPos = GameGui.HudGui.getBeltFieldRect(i).getPosition();
			if (CollisionDetect::isPointInRectangle(mousePosInWorld,beltFieldPos,
				sf::Vector2f(DefaultEqFieldSize,DefaultEqFieldSize)))
			{
				GameGui.HudGui.setHoverForBeltField(i, true);
				if (isMouseClick)
				{
					checkGuiOperations(EquipmentType::Belt, sf::Vector2u(i, 0));
				}
			}

			for (size_t j = 0; j < PlayerFieldsNumber; j++)
			{
				if (GameGui.EquipmentGui.getHoverFromInventoryField(sf::Vector2u(i, j)))
				{
					GameGui.EquipmentGui.setHoverForInventoryField(sf::Vector2u(i, j), false);
				}

				sf::Vector2f fieldPos = GameGui.EquipmentGui.getFieldRect(sf::Vector2u(i, j)).getPosition();
				if (CollisionDetect::isPointInRectangle(mousePosInWorld,fieldPos,
					sf::Vector2f(DefaultEqFieldSize,DefaultEqFieldSize)))
				{
					GameGui.EquipmentGui.setHoverForInventoryField(sf::Vector2u(i, j), true);
					if (isMouseClick)
					{
						checkGuiOperations(EquipmentType::Inventory, sf::Vector2u(i, j));
					}
				}
			}
		}
	}

	if (sf::Mouse::isButtonPressed(sf::Mouse::Right) || !GameGui.getIsEqGuiEnable())
	{
		if (GameGui.getHoldedItem().ItemId != 0)
		{
			LyingItems.pushNewItem(GameClock.getElapsedTime(),
				app.getMousePosInWorld(), GameGui.getHoldedItem());
			GameGui.clearHoldedItem();
		}
	}

	switch (last)
	{
	case mouseWheel::Up:
		GameGui.incrSelectedBeltField();
		break;
	case mouseWheel::Down:
		GameGui.decrSelectedBeltField();
		break;
	case mouseWheel::Stop:
		break;
	default:
		break;
	}

	GameGui.pushKeyState(key);
	GameGui.HudGui.pushNewValuesForHpInfo(200, static_cast<unsigned>(player.getHP()));
	GameGui.HudGui.pushNewValuesForMpInfo(200, static_cast<unsigned>(player.getMP()));
	pushChangesToGui();
}

void Engine::drawWorld(IslandApp & app)
{
	
	sf::Vector2i PlayerPosToTile = Map::getTiledPosition(player.getCharacterCenterPosition());
	sf::RectangleShape TileShape,
		LyingItemShape,
		ObjectShape;
	TileShape.setSize(sf::Vector2f(TILE_SIZE, TILE_SIZE));
	ObjectShape.setSize(RawObjects.ObjectGraphicsSize);
	LyingItemShape.setSize(sf::Vector2f(32, 32));

	for (int i = PlayerPosToTile.x - 30; i < PlayerPosToTile.x + 31; i++)
	{
		for (int j = PlayerPosToTile.y - 30; j < PlayerPosToTile.y + 31; j++)
		{
			if (j < 0 && i < 0) { continue; }
			drawTile(static_cast<sf::Vector2u>(sf::Vector2i(i, j)), *app.getIslandWindow(), TileShape);
			drawObject(static_cast<sf::Vector2u>(sf::Vector2i(i, j)), *app.getIslandWindow(), TileShape);
		}
	}
	sf::Vector2f cameraPos = camera.getCenter() - sf::Vector2f(camera.getSize().x / 2, camera.getSize().y / 2);
	for (size_t i = 0; i < LyingItems.getSize(); i++)
	{
		if (CollisionDetect::isPointInRectangle(LyingItems.getPosition(i), cameraPos, camera.getSize()))
		{
			LyingItemShape.setPosition(LyingItems.getPosition(i));
			LyingItemShape.setTexture(&mediaContainer.getTexture(LyingItems.getItem(i).ItemId,
				TextureContainer::ItemsTextures), true);
			app.draw(LyingItemShape);
		}
	}
}

void Engine::drawPlayerGui(IslandApp & app)
{
	app.draw(GameGui.HudGui.getHudElement(false));
	app.draw(GameGui.HudGui.getHudElement(true));

	sf::Font font;
	font.loadFromFile("Data/Fonts/ariali.ttf");

	sf::Text amountItem;
	amountItem.setCharacterSize(16);
	amountItem.setFont(font);
	amountItem.setFillColor(sf::Color(255, 0, 0, 255));


	if (GameGui.getHoldedItem().ItemId != 0)
	{
		sf::RectangleShape holdedItemRep;
		holdedItemRep.setSize(sf::Vector2f(DefaultEqFieldSize, DefaultEqFieldSize));
		holdedItemRep.setTexture(&mediaContainer.getTexture(GameGui.getHoldedItem().ItemId,
			TextureContainer::ItemsTextures));
		holdedItemRep.setPosition(app.getMousePosInWorld());
		amountItem.setString(std::to_string(GameGui.getHoldedItem().ItemAmount));
		amountItem.setPosition(app.getMousePosInWorld());

		app.draw(holdedItemRep);
		app.draw(amountItem);
	}

	for (size_t i = 0; i < PlayerFieldsNumber; i++)
	{
		app.draw(GameGui.HudGui.getBeltFieldRect(i));

		if (player.getHandInventoryField(i).ItemId != 0)
		{
			app.draw(GameGui.HudGui.getBeltFieldTextureRect(i));
			amountItem.setPosition(GameGui.HudGui.getBeltFieldRect(i).getPosition());
			amountItem.setString(std::to_string(player.getHandInventoryField(i).ItemAmount));
			app.draw(amountItem);
		}
	}

	if (!GameGui.getIsEqGuiEnable()) { return; }

	sf::Vector2u field;
	for (size_t i = 0; i < PlayerFieldsNumber; i++)
	{
		if (i < 3)
		{
			app.draw(GameGui.EquipmentGui.getArmorFieldRect(i));
			if (player.getArmorInventoryField(i).ItemId != 0)
			{
				app.draw(GameGui.EquipmentGui.getArmorTextureRect(i));
			}
		}
		field.x = i;
		for (size_t j = 0; j < PlayerFieldsNumber; j++)
		{
			field.y = j;

			app.draw(GameGui.EquipmentGui.getFieldRect(field));

			if (player.getInventoryField(field).ItemId != 0)
			{
				app.draw(GameGui.EquipmentGui.getTextureRect(field));
				amountItem.setPosition(GameGui.EquipmentGui.getFieldRect(field).getPosition());
				amountItem.setString(std::to_string(player.getInventoryField(field).ItemAmount));
				app.draw(amountItem);
			}
		}
	}
		
}

void Engine::DrawAll(IslandApp &app)
{
	app.getIslandWindow()->setView(camera);

	drawWorld(app);

	drawPlayerGui(app);

	app.draw(*player.getShape());

}