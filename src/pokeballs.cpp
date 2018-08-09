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
#include "pokeballs.h"

extern Game g_game;
extern Professions g_professions;
extern Clans g_clans;
extern ConfigManager g_config;
extern Pokeballs* g_pokeballs;
extern Pokemons g_pokemons;
extern Actions* g_actions;

Pokeballs::Pokeballs()
{
	scriptInterface.initState();
}

Pokeballs::~Pokeballs()
{
	clear();
}

PokeballType* Pokeballs::getPokeballTypeByItemID(const uint16_t id) const
{
	if (!id) {
		return nullptr;
	}

	for (std::map<uint32_t, PokeballType*>::const_iterator it = pokeballs.begin(); it != pokeballs.end(); ++it) {
		if (it->second->getItemId() == id) {
			return it->second;
		}
	}

	return nullptr;
}

PokeballType* Pokeballs::getPokeballTypeByServerID(const uint16_t id) const
{
	if (!id) {
		return nullptr;
	}

	auto it = pokeballs.find(id);
	if (it == pokeballs.end()) {
		return nullptr;
	}
	return it->second;
}

PokeballType* Pokeballs::getPokeballTypeByName(const std::string& name) const
{
	for (std::map<uint32_t, PokeballType*>::const_iterator it = pokeballs.begin(); it != pokeballs.end(); ++it) {
		if (it->second->getName() == name) {
			return it->second;
		}
	}

	return nullptr;
}

void Pokeballs::usePokeball(Player* player, Item* pokeball, const Position& fromPos, const Position& toPos, bool isHotkey) const
{
	player->setNextAction(OTSYS_TIME() + g_config.getNumber(ConfigManager::POKEBALLS_DELAY_INTERVAL));
	player->stopWalk();

	const PokeballType* pokeballType = getPokeballTypeByItemID(pokeball->getID());
	if (!pokeballType) {
		return;
	}

	pokeballType->usePokeball(player, pokeball, fromPos, toPos, isHotkey);
	return;
}

void Pokeballs::clear()
{
	for (const auto& it : pokeballs) {
		delete it.second;
	}
	pokeballs.clear();

	scriptInterface.reInitState();
}

LuaScriptInterface& Pokeballs::getScriptInterface()
{
	return scriptInterface;
}

std::string Pokeballs::getScriptBaseName() const
{
	return "pokeballs";
}

std::string Pokeballs::getScriptPrefixName() const
{
	return "";
}

Event_ptr Pokeballs::getEvent(const std::string& nodeName)
{
	return Event_ptr(new PokeballType(&scriptInterface));	
}

bool Pokeballs::registerEvent(Event_ptr event, const pugi::xml_node&)
{
	PokeballType* pokeballType = static_cast<PokeballType*>(event.release()); //event is guaranteed to be a Pokeball

	auto result = pokeballs.emplace(pokeballType->getServerID(), pokeballType);
	if (!result.second) {
		std::cout << "[Warning - Pokeballs::registerEvent] Duplicate registered pokeball with serverid: " << pokeballType->getServerID() << std::endl;
	}
	return result.second;
}

bool PokeballType::configureEvent(const pugi::xml_node& node)
{
	pugi::xml_attribute attr;
	if (!(attr = node.attribute("serverid"))) {
		std::cout << "[Error - PokeballType::configureEvent] Pokeball without server id." << std::endl;
		return false;
	}
	serverid = pugi::cast<uint16_t>(attr.value());

	if (!(attr = node.attribute("itemid"))) {
		std::cout << "[Error - PokeballType::configureEvent] Pokeball without item id." << std::endl;
		return false;
	}
	itemid = pugi::cast<uint16_t>(attr.value());

	if ((attr = node.attribute("level"))) {
		level = pugi::cast<uint32_t>(attr.value());
	}

	if ((attr = node.attribute("premium"))) {
		premium = pugi::cast<bool>(attr.value());
	}

	if ((attr = node.attribute("name"))) {
		name = pugi::cast<std::string>(attr.value());
	}

	if ((attr = node.attribute("charged"))) {
		charged = pugi::cast<uint16_t>(attr.value());
	}

	if ((attr = node.attribute("discharged"))) {
		discharged = pugi::cast<uint16_t>(attr.value());
	}

	if ((attr = node.attribute("goback"))) {
		goback = pugi::cast<uint16_t>(attr.value());
	}

	if ((attr = node.attribute("catchSuccess"))) {
		catchSuccess = pugi::cast<uint16_t>(attr.value());
	}

	if ((attr = node.attribute("catchFail"))) {
		catchFail = pugi::cast<uint16_t>(attr.value());
	}

	if ((attr = node.attribute("shotEffect"))) {
		shotEffect = pugi::cast<uint16_t>(attr.value());
	}

	if ((attr = node.attribute("rate"))) {
		rate = pugi::cast<double>(attr.value());
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

std::string PokeballType::getScriptEventName() const
{
	return "onUsePokeball";
}

ReturnValue PokeballType::playerPokeballCheck(Player* player, const Position& toPos) const
{
	if (!g_game.canThrowObjectTo(player->getPosition(), toPos)) {
		return RETURNVALUE_CANNOTTHROW;
	}

	Tile* tile = g_game.map.getTile(toPos);
	if (!tile) {
		return RETURNVALUE_CANNOTTHROW;
	}

	if (getReqLevel() > player->getLevel()) {
		return RETURNVALUE_NOTENOUGHLEVEL;
	}

	if (isPremium() && !player->isPremium()) {
		return RETURNVALUE_YOUNEEDPREMIUMACCOUNT;
	}

	Item* corpse = tile->getTopDownItem();
	if (!corpse) {
		return RETURNVALUE_CANONLYUSEPOKEBALLINPOKEMON;
	}

	PokemonType* pokemonType = g_pokemons.getPokemonType(corpse->getPokemonCorpseType());
	if (!pokemonType) {
		return RETURNVALUE_CANONLYUSEPOKEBALLINPOKEMON;
	}

	if (player->getLevel() < level) {
		return RETURNVALUE_NOTENOUGHLEVEL;
	}

	if (!pokemonType->info.isCatchable) {
		return RETURNVALUE_CANNOTCAPTURETHISPOKEMON;
	}

	return RETURNVALUE_NOERROR;
}

bool PokeballType::usePokeball(Player* player, Item* pokeball, const Position& fromPos, const Position& toPos, bool isHotkey) const
{
	ReturnValue ret = playerPokeballCheck(player, toPos);
	if (ret != RETURNVALUE_NOERROR) {
		player->sendCancelMessage(ret);
		return false;
	}

	Item* corpse = g_game.map.getTile(toPos)->getTopDownItem();

	internalUsePokeball(player, pokeball, fromPos, corpse, toPos, isHotkey);
	return true;
}

void PokeballType::internalUsePokeball(Player* player, Item* pokeball, const Position& fromPos, Item* corpse, const Position& toPos, bool isHotkey) const
{
	double currentRate = rate;

	if (scripted) {
		currentRate = executeUsePokeball(player, pokeball, fromPos, corpse, toPos, isHotkey);

		if (currentRate < 0) {
			player->sendCancelMessage(RETURNVALUE_NOTPOSSIBLE);
			return;
		} else if (currentRate == 0) {
			return;
		}
	}

	if (isHotkey) {
		g_actions->showUseHotkeyMessage(player, pokeball, player->getItemTypeCount(pokeball->getID(), -1));
	}

	player->tryCatchPokemon(this, corpse, currentRate, toPos);
	decrementItemCount(pokeball);
}

double PokeballType::executeUsePokeball(Player* player, Item* pokeball, const Position& fromPos, Item* corpse, const Position& toPos, bool isHotkey) const
{
	//onUsePokeball(player, pokeball, corpse)
	if (!scriptInterface->reserveScriptEnv()) {
		std::cout << "[Error - PokeballType::executeUsePokeball] Call stack overflow" << std::endl;
		return false;
	}

	ScriptEnvironment* env = scriptInterface->getScriptEnv();
	env->setScriptId(scriptId, scriptInterface);

	lua_State* L = scriptInterface->getLuaState();

	scriptInterface->pushFunction(scriptId);
	LuaScriptInterface::pushUserdata<Player>(L, player);
	LuaScriptInterface::setMetatable(L, -1, "Player");

	LuaScriptInterface::pushUserdata<Item>(L, pokeball);
	LuaScriptInterface::setItemMetatable(L, -1, pokeball);

	LuaScriptInterface::pushUserdata<PokeballType>(L, g_pokeballs->getPokeballTypeByItemID(itemid));
	LuaScriptInterface::setMetatable(L, -1, "PokeballType");

	LuaScriptInterface::pushPosition(L, fromPos);

	LuaScriptInterface::pushUserdata<Item>(L, corpse);
	LuaScriptInterface::setItemMetatable(L, -1, corpse);

	LuaScriptInterface::pushPosition(L, toPos);
	
	LuaScriptInterface::pushBoolean(L, isHotkey);

	return scriptInterface->callPokeballFunction(7);
}

void PokeballType::decrementItemCount(Item* item)
{
	uint16_t count = item->getItemCount();
	if (count > 1) {
		g_game.transformItem(item, item->getID(), count - 1);
	} else {
		g_game.internalRemoveItem(item);
	}
}