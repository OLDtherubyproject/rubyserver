/**
 * The Ruby Server - a free and open-source Pokémon MMORPG server emulator
 * Copyright (C) 2018  Mark Samman (TFS) <mark.samman@gmail.com>
 *                     Leandro Matheus <kesuhige@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#include "otpch.h"

#include "combat.h"
#include "configmanager.h"
#include "game.h"
#include "pugicast.h"
#include "actions.h"
#include "foods.h"
#include "events.h"

extern Game g_game;
extern Professions g_professions;
extern Clans g_clans;
extern ConfigManager g_config;
extern Foods* g_foods;
extern Pokemons g_pokemons;
extern Actions* g_actions;
extern Events* g_events;

Foods::Foods()
{
	scriptInterface.initState();
}

Foods::~Foods()
{
	clear();
}

FoodType* Foods::getFoodTypeByItemID(const uint16_t id) const
{
	if (!id) {
		return nullptr;
	}

	for (std::map<uint32_t, FoodType*>::const_iterator it = foods.begin(); it != foods.end(); ++it) {
		if (it->second->getItemId() == id) {
			return it->second;
		}
	}

	return nullptr;
}

FoodType* Foods::getFoodTypeByName(const std::string& name) const
{
	for (std::map<uint32_t, FoodType*>::const_iterator it = foods.begin(); it != foods.end(); ++it) {
		if (strcasecmp(Item::items.getItemType(it->second->getItemId()).name.c_str(), name.c_str()) == 1) {
			return it->second;
		}
	}

	return nullptr;
}

void Foods::useFood(Player* player, const Position& fromPos, const Position& toPos, Item* item, bool isHotkey, Creature* creature /*=nullptr*/) const
{
	player->setNextAction(OTSYS_TIME() + g_config.getNumber(ConfigManager::FOODS_DELAY_INTERVAL));
	player->stopWalk();

	const FoodType* foodType = getFoodTypeByItemID(item->getID());
	if (!foodType) {
		return;
	}

	foodType->useFood(player, fromPos, toPos, item, isHotkey, creature);
	return;
}

void Foods::clear()
{
	for (const auto& it : foods) {
		delete it.second;
	}
	foods.clear();

	scriptInterface.reInitState();
}

LuaScriptInterface& Foods::getScriptInterface()
{
	return scriptInterface;
}

std::string Foods::getScriptBaseName() const
{
	return "foods";
}

std::string Foods::getScriptPrefixName() const
{
	return "";
}

Event_ptr Foods::getEvent(const std::string& nodeName)
{
	return Event_ptr(new FoodType(&scriptInterface));	
}

bool Foods::registerEvent(Event_ptr event, const pugi::xml_node&)
{
	FoodType* foodType = static_cast<FoodType*>(event.release()); //event is guaranteed to be a Food

	auto result = foods.emplace(foodType->getItemId(), foodType);
	if (!result.second) {
		std::cout << "[Warning - Foods::registerEvent] Duplicate registered food with itemid: " << foodType->getItemId() << std::endl;
	}
	return result.second;
}

bool FoodType::configureEvent(const pugi::xml_node& node)
{
	pugi::xml_attribute attr;
	if (!(attr = node.attribute("itemid"))) {
		std::cout << "[Error - FoodType::configureEvent] Food without server id." << std::endl;
		return false;
	}
	itemid = pugi::cast<uint16_t>(attr.value());

	if (!(attr = node.attribute("itemid"))) {
		std::cout << "[Error - FoodType::configureEvent] Food without item id." << std::endl;
		return false;
	}
	itemid = pugi::cast<uint16_t>(attr.value());

	if ((attr = node.attribute("level"))) {
		level = pugi::cast<uint32_t>(attr.value());
	}

	if ((attr = node.attribute("premium"))) {
		premium = pugi::cast<bool>(attr.value());
	}

	if ((attr = node.attribute("sound"))) {
		sound = pugi::cast<std::string>(attr.value());
	}

	if ((attr = node.attribute("regen"))) {
		regen = pugi::cast<uint16_t>(attr.value());
	}

	if ((attr = node.attribute("for"))) {
		std::string tmpStrValue = asLowerCaseString(attr.as_string());
		uint16_t tmpInt = pugi::cast<uint16_t>(attr.value());
		if (tmpStrValue == "player" || tmpInt == 1) {
			targets = 1;
		} else if (tmpStrValue == "pokemon" || tmpInt == 2) {
			targets = 2;
		} else if (tmpStrValue == "everyone" || tmpInt == 3) {
			targets = 3;
		} else {
			std::cout << "[Warning - FoodType::configureEvent] Unknown target " << attr.as_string() << ". itemid: " << itemid << std::endl;
		}
	}

	for (auto natureNode : node.children()) {
		uint16_t natureId = 0;
		bool likes = false;
		if ((attr = natureNode.attribute("id"))) {
			uint16_t tmpInt = pugi::cast<uint16_t>(attr.value());

			if (tmpInt >= 1 && tmpInt <= NATURE_LAST) {
				natureId = tmpInt;
			} else {
				std::cout << "[Warning - FoodType::configureEvent] Unknown nature id " << attr.as_string() << std::endl;
			}
		} else {
			std::cout << "[Warning - FoodType::configureEvent]] Missing nature id. " << std::endl;
		}

		if ((attr = natureNode.attribute("likes"))) {
			likes = attr.as_bool();
		} else {
			std::cout << "[Warning - FoodType::configureEvent] Missing nature likes. " << std::endl;
		}

		natureLikes[natureId] = likes;
	}

	uint32_t wieldInfo = 0;
	if (getReqLevel() > 0) {
		wieldInfo |= WIELDINFO_LEVEL;
	}

	if (isPremium()) {
		wieldInfo |= WIELDINFO_PREMIUM;
	}

	if (wieldInfo != 0) {
		ItemType& it = Item::items.getItemType(getItemId());
		it.wieldInfo = wieldInfo;
		it.minReqLevel = getReqLevel();
	}

	return true;
}

std::string FoodType::getScriptEventName() const
{
	return "onUseFood";
}

ReturnValue FoodType::playerFoodCheck(Player* player, const Position& toPos, Creature* creature /*= nullptr*/) const
{
	if (!g_game.canThrowObjectTo(player->getPosition(), toPos)) {
		return RETURNVALUE_CANNOTTHROW;
	}

	if (getReqLevel() > player->getLevel()) {
		return RETURNVALUE_NOTENOUGHLEVEL;
	}

	if (isPremium() && !player->isPremium()) {
		return RETURNVALUE_YOUNEEDPREMIUMACCOUNT;
	}

	if (!creature) {
		if (canEveryoneEat()) {
			return RETURNVALUE_CANONLYUSEFOODINYOURPOKEMONORINYOURSELF;
		} else if (canPokemonEat()) {
			return RETURNVALUE_CANONLYUSEFOODINYOURPOKEMON;
		} else if (canPlayerEat()) {
			return RETURNVALUE_CANONLYUSEFOODINYOURSELF;
		}
	}

	if (Pokemon* targetPokemon = creature->getPokemon()) {
		if (targetPokemon != player->getHisPokemon()) {
			return RETURNVALUE_CANONLYUSEFOODINYOURPOKEMON;
		}

		if (!getNatureLikes(targetPokemon->getNature())) {
			return RETURNVALUE_YOURPOKEMONDONOTLIKEIT;
		}
	} else if (Player* targetPlayer = creature->getPlayer()) {
		if (targetPlayer != player) {
			return RETURNVALUE_CANONLYUSEFOODINYOURSELF;
		}
	}

	return RETURNVALUE_NOERROR;
}

bool FoodType::useFood(Player* player, const Position& fromPos, const Position& toPos, Item* item, bool isHotkey, Creature* creature /* = nullptr*/) const
{
	ReturnValue ret = playerFoodCheck(player, toPos, creature);
	if (ret != RETURNVALUE_NOERROR) {
		if (ret == RETURNVALUE_YOURPOKEMONDONOTLIKEIT) {
			player->sendPokemonEffect(CONST_ME_EMOT_SEASICK);
			player->sendPokemonTextMessage("Grrrrr");
		}

		player->sendCancelMessage(ret);
		return false;
	}

	internalUseFood(player, fromPos, toPos, item, isHotkey, creature);
	return true;
}

void FoodType::internalUseFood(Player* player, const Position& fromPos, const Position& toPos, Item* item, bool isHotkey, Creature* creature /* = nullptr*/) const
{
	if (scripted) {
		if (!executeUseFood(player, fromPos, toPos, item, isHotkey, creature)) {
			return;
		}
	}

	if (isHotkey) {
		g_actions->showUseHotkeyMessage(player, item, player->getItemTypeCount(item->getID(), -1));
	}

	if (!g_events->eventPlayerOnFeed(player, creature, this)) {
		return;
	}

	if (creature->getPokemon() && !g_events->eventPlayerOnFeedPokemon(player, creature->getPokemon(), this)) {
		return;
	} else if (creature->getPlayer() && !g_events->eventPlayerOnFeedHimself(player, this)) {
		return;
	}

	if (creature->feed(this)) {
		decrementItemCount(item);
	}	
}

bool FoodType::executeUseFood(Player* player, const Position& fromPos, const Position& toPos, Item* item, bool isHotkey, Creature* creature /* = nullptr*/) const
{
	//onUseFood(player, fromPos, toPos, item, foodType, creature)
	if (!scriptInterface->reserveScriptEnv()) {
		std::cout << "[Error - FoodType::executeUseFood] Call stack overflow" << std::endl;
		return false;
	}

	ScriptEnvironment* env = scriptInterface->getScriptEnv();
	env->setScriptId(scriptId, scriptInterface);

	lua_State* L = scriptInterface->getLuaState();

	scriptInterface->pushFunction(scriptId);
	LuaScriptInterface::pushUserdata<Player>(L, player);
	LuaScriptInterface::setMetatable(L, -1, "Player");

	LuaScriptInterface::pushUserdata<Item>(L, item);
	LuaScriptInterface::setItemMetatable(L, -1, item);

	LuaScriptInterface::pushUserdata<FoodType>(L, g_foods->getFoodTypeByItemID(itemid));
	LuaScriptInterface::setMetatable(L, -1, "FoodType");

	LuaScriptInterface::pushPosition(L, fromPos);

	LuaScriptInterface::pushPosition(L, toPos);

	LuaScriptInterface::pushBoolean(L, isHotkey);

	LuaScriptInterface::pushUserdata<Creature>(L, creature);
	LuaScriptInterface::setCreatureMetatable(L, -1, creature);

	return scriptInterface->callFunction(7);
}

void FoodType::decrementItemCount(Item* item)
{
	uint16_t count = item->getItemCount();
	if (count > 1) {
		g_game.transformItem(item, item->getID(), count - 1);
	} else {
		g_game.internalRemoveItem(item);
	}
}

bool FoodType::getNatureLikes(Natures_t nature) const
{
	if (nature > NATURE_LAST) {
		return false;
	}

	return natureLikes[nature];
}