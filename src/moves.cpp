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
#include "pokemon.h"
#include "pugicast.h"
#include "moves.h"

extern Game g_game;
extern Moves* g_moves;
extern Pokemons g_pokemons;
extern Professions g_professions;
extern Clans g_clans;
extern ConfigManager g_config;
extern LuaEnvironment g_luaEnvironment;

Moves::Moves()
{
	scriptInterface.initState();
}

Moves::~Moves()
{
	clear();
}

void Moves::clear()
{
	moves.clear();

	scriptInterface.reInitState();
}

LuaScriptInterface& Moves::getScriptInterface()
{
	return scriptInterface;
}

std::string Moves::getScriptBaseName() const
{
	return "moves";
}

std::string Moves::getScriptPrefixName() const
{
	return "type";
}

Event_ptr Moves::getEvent(const std::string& nodeName)
{
	return Event_ptr(new Move(&scriptInterface));
}

bool Moves::registerEvent(Event_ptr event, const pugi::xml_node&)
{
	Move* move = dynamic_cast<Move*>(event.get());
	if (move) {
		std::string moveName = move->getName();
		auto result = moves.emplace(moveName, std::move(*move));
		if (!result.second) {
			std::cout << "[Warning - Moves::registerEvent] Duplicate registered move with name: " << moveName << std::endl;
		}
		return result.second;
	}

	return false;
}

Move* Moves::getMove(uint16_t moveId)
{
	for (auto& it : moves) {
		if (it.second.getId() == moveId) {
			return &it.second;
		}
	}
	return nullptr;
}

Move* Moves::getMoveByName(const std::string& name)
{
	for (auto& it : moves) {
		if (strcasecmp(it.second.getName().c_str(), name.c_str()) == 0) {
			return &it.second;
		}
	}
	return nullptr;
}

Position Moves::getCasterPosition(Creature* creature, Direction dir)
{
	return getNextPosition(dir, creature->getPosition());
}

bool Move::configureMove(const pugi::xml_node& node)
{
	pugi::xml_attribute nameAttribute = node.attribute("name");
	if (!nameAttribute) {
		std::cout << "[Error - Move::configureMove] Move without name" << std::endl;
		return false;
	}

	name = nameAttribute.as_string();

	pugi::xml_attribute attr;
	if ((attr = node.attribute("moveid"))) {
		moveId = pugi::cast<uint16_t>(attr.value());
	} else {
		std::cout << "[Error - Move::configureMove] Move without id" << std::endl;
		return false;
	}
	
	if ((attr = node.attribute("category"))) {
		std::string tmpStr = asLowerCaseString(attr.as_string());
		if (tmpStr == "physical" || tmpStr == "0") {
			category = MOVECATEGORY_PHYSICAL;
		} else if (tmpStr == "special" || tmpStr == "1") {
			category = MOVECATEGORY_SPECIAL;
		} else if (tmpStr == "status" || tmpStr == "2") {
			category = MOVECATEGORY_STATUS;
		} else {
			std::cout << "[Warning - Move::configureMove] Unknown category: " << attr.as_string() << std::endl;
		}
	}

	if ((attr = node.attribute("type"))) {
		std::string tmpStr = asLowerCaseString(attr.as_string());
		if (tmpStr == "normal" || tmpStr == "0") {
			type = MOVETYPE_NORMAL;
		} else if (tmpStr == "fighting" || tmpStr == "1") {
			type = MOVETYPE_FIGHTING;
		} else if (tmpStr == "flying" || tmpStr == "2") {
			type = MOVETYPE_FLYING;
		} else if (tmpStr == "poison" || tmpStr == "3") {
			type = MOVETYPE_POISON;
		} else if (tmpStr == "ground" || tmpStr == "4") {
			type = MOVETYPE_GROUND;
		} else if (tmpStr == "rock" || tmpStr == "5") {
			type = MOVETYPE_ROCK;
		} else if (tmpStr == "bug" || tmpStr == "6") {
			type = MOVETYPE_BUG;
		} else if (tmpStr == "ghost" || tmpStr == "7") {
			type = MOVETYPE_GHOST;
		} else if (tmpStr == "steel" || tmpStr == "8") {
			type = MOVETYPE_STEEL;
		} else if (tmpStr == "fire" || tmpStr == "9") {
			type = MOVETYPE_FIRE;
		} else if (tmpStr == "water" || tmpStr == "10") {
			type = MOVETYPE_WATER;
		} else if (tmpStr == "grass" || tmpStr == "11") {
			type = MOVETYPE_GRASS;
		} else if (tmpStr == "electric" || tmpStr == "12") {
			type = MOVETYPE_ELECTRIC;
		} else if (tmpStr == "psychic" || tmpStr == "13") {
			type = MOVETYPE_PSYCHIC;
		} else if (tmpStr == "ice" || tmpStr == "14") {
			type = MOVETYPE_ICE;
		} else if (tmpStr == "dragon" || tmpStr == "15") {
			type = MOVETYPE_DRAGON;
		} else if (tmpStr == "dark" || tmpStr == "16") {
			type = MOVETYPE_DARK;
		} else if (tmpStr == "fairy" || tmpStr == "17") {
			type = MOVETYPE_FAIRY;
		} else {
			std::cout << "[Warning - Move::configureMove] Unknown category: " << attr.as_string() << std::endl;
		}
	}

	if ((attr = node.attribute("level")) || (attr = node.attribute("lvl"))) {
		level = pugi::cast<uint32_t>(attr.value());
	}

	if ((attr = node.attribute("range"))) {
		range = pugi::cast<int32_t>(attr.value());
	}

	if ((attr = node.attribute("cooldown")) || (attr = node.attribute("exhaustion"))) {
		cooldown = pugi::cast<uint32_t>(attr.value());
	}

	if ((attr = node.attribute("ignoreSleep"))) {
		ignoreSleep = attr.as_bool();
	}

	if ((attr = node.attribute("accuracy"))) {
		accuracy = pugi::cast<uint32_t>(attr.value());
	}

	if ((attr = node.attribute("premium")) || (attr = node.attribute("prem"))) {
		premium = attr.as_bool();
	}

	if ((attr = node.attribute("enabled"))) {
		enabled = attr.as_bool();
	}

	if ((attr = node.attribute("needtarget"))) {
		needTarget = attr.as_bool();
	}

	if ((attr = node.attribute("selftarget"))) {
		selfTarget = attr.as_bool();
	}

	if ((attr = node.attribute("blocking"))) {
		blockingSolid = attr.as_bool();
		blockingCreature = blockingSolid;
	}

	if ((attr = node.attribute("blocktype"))) {
		std::string tmpStrValue = asLowerCaseString(attr.as_string());
		if (tmpStrValue == "all") {
			blockingSolid = true;
			blockingCreature = true;
		} else if (tmpStrValue == "solid") {
			blockingSolid = true;
		} else if (tmpStrValue == "creature") {
			blockingCreature = true;
		} else {
			std::cout << "[Warning - Move::configureMove] Blocktype \"" << attr.as_string() << "\" does not exist." << std::endl;
		}
	}

	if ((attr = node.attribute("aggressive"))) {
		aggressive = booleanString(attr.as_string());
	}
	return true;
}

std::string Move::getScriptEventName() const
{
	return "onCastMove";
}

bool Move::configureEvent(const pugi::xml_node& node)
{
	if (!Move::configureMove(node)) {
		return false;
	}

	pugi::xml_attribute attr;
	if ((attr = node.attribute("params"))) {
		hasParam = attr.as_bool();
	}

	if ((attr = node.attribute("playernameparam"))) {
		hasPlayerNameParam = attr.as_bool();
	}

	if ((attr = node.attribute("direction"))) {
		needDirection = attr.as_bool();
	} else if ((attr = node.attribute("casterTargetOrDirection"))) {
		casterTargetOrDirection = attr.as_bool();
	}

	if ((attr = node.attribute("blockwalls"))) {
		checkLineOfSight = attr.as_bool();
	}
	return true;
}

bool Move::canThrowMove(const Creature* creature, const Creature* target) const
{
	const Position& fromPos = creature->getPosition();
	const Position& toPos = target->getPosition();
	if (fromPos.z != toPos.z ||
	        (range == -1 && !g_game.canThrowObjectTo(fromPos, toPos, checkLineOfSight)) ||
	        (range != -1 && !g_game.canThrowObjectTo(fromPos, toPos, checkLineOfSight, range, range))) {
		return false;
	}
	return true;
}

PokemonType_t Moves::MoveTypeToPokemonType(MoveType_t type)
{
	switch (type) {
		case MOVETYPE_NORMAL:
			return TYPE_NORMAL;

		case MOVETYPE_FIGHTING:
			return TYPE_FIGHTING;

		case MOVETYPE_FLYING:
			return TYPE_FLYING;

		case MOVETYPE_POISON:
			return TYPE_POISON;

		case MOVETYPE_GROUND:
			return TYPE_GROUND;

		case MOVETYPE_ROCK:
			return TYPE_ROCK;

		case MOVETYPE_BUG:
			return TYPE_BUG;

		case MOVETYPE_GHOST:
			return TYPE_GHOST;

		case MOVETYPE_STEEL:
			return TYPE_STEEL;

		case MOVETYPE_FIRE:
			return TYPE_FIRE;

		case MOVETYPE_WATER:
			return TYPE_WATER;

		case MOVETYPE_GRASS:
			return TYPE_GRASS;

		case MOVETYPE_ELECTRIC:
			return TYPE_ELECTRIC;

		case MOVETYPE_PSYCHIC:
			return TYPE_PSYCHIC;

		case MOVETYPE_ICE:
			return TYPE_ICE;

		case MOVETYPE_DRAGON:
			return TYPE_DRAGON;

		case MOVETYPE_DARK:
			return TYPE_DARK;

		case MOVETYPE_FAIRY:
			return TYPE_FAIRY;
			
		default:
			break;
	}

	return TYPE_NONE;
}

MoveType_t Moves::PokemonTypeToMoveType(PokemonType_t type)
{
	switch (type) {
		case TYPE_NORMAL:
			return MOVETYPE_NORMAL;

		case TYPE_FIGHTING:
			return MOVETYPE_FIGHTING;

		case TYPE_FLYING:
			return MOVETYPE_FLYING;

		case TYPE_POISON:
			return MOVETYPE_POISON;

		case TYPE_GROUND:
			return MOVETYPE_GROUND;

		case TYPE_ROCK:
			return MOVETYPE_ROCK;

		case TYPE_BUG:
			return MOVETYPE_BUG;

		case TYPE_GHOST:
			return MOVETYPE_GHOST;

		case TYPE_STEEL:
			return MOVETYPE_STEEL;

		case TYPE_FIRE:
			return MOVETYPE_FIRE;

		case TYPE_WATER:
			return MOVETYPE_WATER;

		case TYPE_GRASS:
			return MOVETYPE_GRASS;

		case TYPE_ELECTRIC:
			return MOVETYPE_ELECTRIC;

		case TYPE_PSYCHIC:
			return MOVETYPE_PSYCHIC;

		case TYPE_ICE:
			return MOVETYPE_ICE;

		case TYPE_DRAGON:
			return MOVETYPE_DRAGON;

		case TYPE_DARK:
			return MOVETYPE_DARK;

		case TYPE_FAIRY:
			return MOVETYPE_FAIRY;
			
		default:
			break;
	}

	return MOVETYPE_NONE;
}

void Move::serialize(PropWriteStream& propWriteStream)
{
	propWriteStream.write<uint16_t>(getId());
}

bool Move::castMove(Creature* creature)
{
	if (normal_random(1, 100) > getAccuracy()) {
		return false;
	}

	LuaVariant var;

	if (casterTargetOrDirection) {
		Creature* target = creature->getAttackedCreature();
		if (target && target->getHealth() > 0) {
			if (!canThrowMove(creature, target)) {
				return false;
			}

			var.type = VARIANT_NUMBER;
			var.number = target->getID();
			return internalCastMove(creature, var);
		}

		return false;
	} else if (needDirection) {
		var.type = VARIANT_POSITION;
		var.pos = Moves::getCasterPosition(creature, creature->getDirection());
	} else {
		var.type = VARIANT_POSITION;
		var.pos = creature->getPosition();
	}

	return internalCastMove(creature, var);
}

bool Move::castMove(Creature* creature, Creature* target)
{
	if (normal_random(1, 100) > getAccuracy()) {
		return false;
	}

	if (needTarget) {
		LuaVariant var;
		var.type = VARIANT_NUMBER;
		var.number = target->getID();
		return internalCastMove(creature, var);
	} else {
		return castMove(creature);
	}
}

bool Move::internalCastMove(Creature* creature, const LuaVariant& var)
{
	return executeCastMove(creature, var);
}

bool Move::executeCastMove(Creature* creature, const LuaVariant& var)
{
	//onCastMove(Creature, var)
	if (!scriptInterface->reserveScriptEnv()) {
		std::cout << "[Error - Move::executeCastMove] Call stack overflow" << std::endl;
		return false;
	}

	ScriptEnvironment* env = scriptInterface->getScriptEnv();
	env->setScriptId(scriptId, scriptInterface);

	lua_State* L = scriptInterface->getLuaState();

	scriptInterface->pushFunction(scriptId);

	LuaScriptInterface::pushUserdata<Creature>(L, creature);
	LuaScriptInterface::setCreatureMetatable(L, -1, creature);

	LuaScriptInterface::pushVariant(L, var);

	return scriptInterface->callFunction(2);
}