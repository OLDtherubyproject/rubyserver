/**
 * * The Ruby Server - a free and open-source Pokémon MMORPG server emulator
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
extern Vocations g_vocations;
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

TalkActionResult_t Moves::playerSayMove(Player* player, std::string& words)
{
	std::string str_words = words;

	//strip trailing spaces
	trimString(str_words);

	InstantMove* instantMove = getInstantMove(str_words);
	if (!instantMove) {
		return TALKACTION_CONTINUE;
	}

	std::string param;

	if (instantMove->getHasParam()) {
		size_t moveLen = instantMove->getWords().length();
		size_t paramLen = str_words.length() - moveLen;
		std::string paramText = str_words.substr(moveLen, paramLen);
		if (!paramText.empty() && paramText.front() == ' ') {
			size_t loc1 = paramText.find('"', 1);
			if (loc1 != std::string::npos) {
				size_t loc2 = paramText.find('"', loc1 + 1);
				if (loc2 == std::string::npos) {
					loc2 = paramText.length();
				} else if (paramText.find_last_not_of(' ') != loc2) {
					return TALKACTION_CONTINUE;
				}

				param = paramText.substr(loc1 + 1, loc2 - loc1 - 1);
			} else {
				trimString(paramText);
				loc1 = paramText.find(' ', 0);
				if (loc1 == std::string::npos) {
					param = paramText;
				} else {
					return TALKACTION_CONTINUE;
				}
			}
		}
	}

	if (instantMove->playerCastInstant(player, param)) {
		words = instantMove->getWords();

		if (instantMove->getHasParam() && !param.empty()) {
			words += " \"" + param + "\"";
		}

		return TALKACTION_BREAK;
	}

	return TALKACTION_FAILED;
}

void Moves::clear()
{
	runes.clear();
	instants.clear();

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

Event_ptr Moves::getEvent(const std::string& nodeName)
{
	if (strcasecmp(nodeName.c_str(), "rune") == 0) {
		return Event_ptr(new RuneMove(&scriptInterface));
	} else if (strcasecmp(nodeName.c_str(), "instant") == 0) {
		return Event_ptr(new InstantMove(&scriptInterface));
	}
	return nullptr;
}

bool Moves::registerEvent(Event_ptr event, const pugi::xml_node&)
{
	InstantMove* instant = dynamic_cast<InstantMove*>(event.get());
	if (instant) {
		auto result = instants.emplace(instant->getWords(), std::move(*instant));
		if (!result.second) {
			std::cout << "[Warning - Moves::registerEvent] Duplicate registered instant move with words: " << instant->getWords() << std::endl;
		}
		return result.second;
	}

	RuneMove* rune = dynamic_cast<RuneMove*>(event.get());
	if (rune) {
		auto result = runes.emplace(rune->getRuneItemId(), std::move(*rune));
		if (!result.second) {
			std::cout << "[Warning - Moves::registerEvent] Duplicate registered rune with id: " << rune->getRuneItemId() << std::endl;
		}
		return result.second;
	}

	return false;
}

Move* Moves::getMoveByName(const std::string& name)
{
	Move* move = getRuneMoveByName(name);
	if (!move) {
		move = getInstantMoveByName(name);
	}
	return move;
}

RuneMove* Moves::getRuneMove(uint32_t id)
{
	auto it = runes.find(id);
	if (it == runes.end()) {
		return nullptr;
	}
	return &it->second;
}

RuneMove* Moves::getRuneMoveByName(const std::string& name)
{
	for (auto& it : runes) {
		if (strcasecmp(it.second.getName().c_str(), name.c_str()) == 0) {
			return &it.second;
		}
	}
	return nullptr;
}

InstantMove* Moves::getInstantMove(const std::string& words)
{
	InstantMove* result = nullptr;

	for (auto& it : instants) {
		const std::string& instantMoveWords = it.second.getWords();
		size_t moveLen = instantMoveWords.length();
		if (strncasecmp(instantMoveWords.c_str(), words.c_str(), moveLen) == 0) {
			if (!result || moveLen > result->getWords().length()) {
				result = &it.second;
				if (words.length() == moveLen) {
					break;
				}
			}
		}
	}

	if (result) {
		const std::string& resultWords = result->getWords();
		if (words.length() > resultWords.length()) {
			if (!result->getHasParam()) {
				return nullptr;
			}

			size_t moveLen = resultWords.length();
			size_t paramLen = words.length() - moveLen;
			if (paramLen < 2 || words[moveLen] != ' ') {
				return nullptr;
			}
		}
		return result;
	}
	return nullptr;
}

InstantMove* Moves::getInstantMoveById(uint32_t moveId)
{
	auto it = std::next(instants.begin(), std::min<uint32_t>(moveId, instants.size()));
	if (it != instants.end()) {
		return &it->second;
	}
	return nullptr;
}

InstantMove* Moves::getInstantMoveByName(const std::string& name)
{
	for (auto& it : instants) {
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

CombatMove::CombatMove(Combat* combat, bool needTarget, bool needDirection) :
	Event(&g_moves->getScriptInterface()),
	combat(combat),
	needDirection(needDirection),
	needTarget(needTarget)
{}

CombatMove::~CombatMove()
{
	if (!scripted) {
		delete combat;
	}
}

bool CombatMove::loadScriptCombat()
{
	combat = g_luaEnvironment.getCombatObject(g_luaEnvironment.lastCombatId);
	return combat != nullptr;
}

bool CombatMove::castMove(Creature* creature)
{
	if (scripted) {
		LuaVariant var;
		var.type = VARIANT_POSITION;

		if (needDirection) {
			var.pos = Moves::getCasterPosition(creature, creature->getDirection());
		} else {
			var.pos = creature->getPosition();
		}

		return executeCastMove(creature, var);
	}

	Position pos;
	if (needDirection) {
		pos = Moves::getCasterPosition(creature, creature->getDirection());
	} else {
		pos = creature->getPosition();
	}

	combat->doCombat(creature, pos);
	return true;
}

bool CombatMove::castMove(Creature* creature, Creature* target)
{
	if (scripted) {
		LuaVariant var;

		if (combat->hasArea()) {
			var.type = VARIANT_POSITION;

			if (needTarget) {
				var.pos = target->getPosition();
			} else if (needDirection) {
				var.pos = Moves::getCasterPosition(creature, creature->getDirection());
			} else {
				var.pos = creature->getPosition();
			}
		} else {
			var.type = VARIANT_NUMBER;
			var.number = target->getID();
		}
		return executeCastMove(creature, var);
	}

	if (combat->hasArea()) {
		if (needTarget) {
			combat->doCombat(creature, target->getPosition());
		} else {
			return castMove(creature);
		}
	} else {
		combat->doCombat(creature, target);
	}
	return true;
}

bool CombatMove::executeCastMove(Creature* creature, const LuaVariant& var)
{
	//onCastMove(creature, var)
	if (!scriptInterface->reserveScriptEnv()) {
		std::cout << "[Error - CombatMove::executeCastMove] Call stack overflow" << std::endl;
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

bool Move::configureMove(const pugi::xml_node& node)
{
	pugi::xml_attribute nameAttribute = node.attribute("name");
	if (!nameAttribute) {
		std::cout << "[Error - Move::configureMove] Move without name" << std::endl;
		return false;
	}

	name = nameAttribute.as_string();

	static const char* reservedList[] = {
		"melee",
		"physical",
		"poison",
		"fire",
		"energy",
		"drown",
		"lifedrain",
		"manadrain",
		"healing",
		"speed",
		"outfit",
		"invisible",
		"drunk",
		"firefield",
		"poisonfield",
		"energyfield",
		"firecondition",
		"poisoncondition",
		"energycondition",
		"drowncondition",
		"freezecondition",
		"cursecondition",
		"dazzlecondition"
	};

	//static size_t size = sizeof(reservedList) / sizeof(const char*);
	//for (size_t i = 0; i < size; ++i) {
	for (const char* reserved : reservedList) {
		if (strcasecmp(reserved, name.c_str()) == 0) {
			std::cout << "[Error - Move::configureMove] Move is using a reserved name: " << reserved << std::endl;
			return false;
		}
	}

	pugi::xml_attribute attr;
	if ((attr = node.attribute("moveid"))) {
		moveId = pugi::cast<uint16_t>(attr.value());
	}

	if ((attr = node.attribute("group"))) {
		std::string tmpStr = asLowerCaseString(attr.as_string());
		if (tmpStr == "none" || tmpStr == "0") {
			group = MOVEGROUP_NONE;
		} else if (tmpStr == "attack" || tmpStr == "1") {
			group = MOVEGROUP_ATTACK;
		} else if (tmpStr == "healing" || tmpStr == "2") {
			group = MOVEGROUP_HEALING;
		} else if (tmpStr == "support" || tmpStr == "3") {
			group = MOVEGROUP_SUPPORT;
		} else if (tmpStr == "special" || tmpStr == "4") {
			group = MOVEGROUP_SPECIAL;
		} else {
			std::cout << "[Warning - Move::configureMove] Unknown group: " << attr.as_string() << std::endl;
		}
	}

	if ((attr = node.attribute("groupcooldown"))) {
		groupCooldown = pugi::cast<uint32_t>(attr.value());
	}

	if ((attr = node.attribute("secondarygroup"))) {
		std::string tmpStr = asLowerCaseString(attr.as_string());
		if (tmpStr == "none" || tmpStr == "0") {
			secondaryGroup = MOVEGROUP_NONE;
		} else if (tmpStr == "attack" || tmpStr == "1") {
			secondaryGroup = MOVEGROUP_ATTACK;
		} else if (tmpStr == "healing" || tmpStr == "2") {
			secondaryGroup = MOVEGROUP_HEALING;
		} else if (tmpStr == "support" || tmpStr == "3") {
			secondaryGroup = MOVEGROUP_SUPPORT;
		} else if (tmpStr == "special" || tmpStr == "4") {
			secondaryGroup = MOVEGROUP_SPECIAL;
		} else {
			std::cout << "[Warning - Move::configureMove] Unknown secondarygroup: " << attr.as_string() << std::endl;
		}
	}

	if ((attr = node.attribute("secondarygroupcooldown"))) {
		secondaryGroupCooldown = pugi::cast<uint32_t>(attr.value());
	}

	if ((attr = node.attribute("level")) || (attr = node.attribute("lvl"))) {
		level = pugi::cast<uint32_t>(attr.value());
	}

	if ((attr = node.attribute("magiclevel")) || (attr = node.attribute("maglv"))) {
		magLevel = pugi::cast<uint32_t>(attr.value());
	}

	if ((attr = node.attribute("mana"))) {
		mana = pugi::cast<uint32_t>(attr.value());
	}

	if ((attr = node.attribute("manapercent"))) {
		manaPercent = pugi::cast<uint32_t>(attr.value());
	}

	if ((attr = node.attribute("soul"))) {
		soul = pugi::cast<uint32_t>(attr.value());
	}

	if ((attr = node.attribute("range"))) {
		range = pugi::cast<int32_t>(attr.value());
	}

	if ((attr = node.attribute("cooldown")) || (attr = node.attribute("exhaustion"))) {
		cooldown = pugi::cast<uint32_t>(attr.value());
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

	if ((attr = node.attribute("needweapon"))) {
		needWeapon = attr.as_bool();
	}

	if ((attr = node.attribute("selftarget"))) {
		selfTarget = attr.as_bool();
	}

	if ((attr = node.attribute("needlearn"))) {
		learnable = attr.as_bool();
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

	if (group == MOVEGROUP_NONE) {
		group = (aggressive ? MOVEGROUP_ATTACK : MOVEGROUP_HEALING);
	}

	for (auto vocationNode : node.children()) {
		if (!(attr = vocationNode.attribute("name"))) {
			continue;
		}

		int32_t vocationId = g_vocations.getVocationId(attr.as_string());
		if (vocationId != -1) {
			attr = vocationNode.attribute("showInDescription");
			vocMoveMap[vocationId] = !attr || attr.as_bool();
		} else {
			std::cout << "[Warning - Move::configureMove] Wrong vocation name: " << attr.as_string() << std::endl;
		}
	}
	return true;
}

bool Move::playerMoveCheck(Player* player) const
{
	if (player->hasFlag(PlayerFlag_CannotUseMoves)) {
		return false;
	}

	if (player->hasFlag(PlayerFlag_IgnoreMoveCheck)) {
		return true;
	}

	if (!enabled) {
		return false;
	}

	if (aggressive && (range < 1 || (range > 0 && !player->getAttackedCreature())) && player->getSkull() == SKULL_BLACK) {
		player->sendCancelMessage(RETURNVALUE_NOTPOSSIBLE);
		return false;
	}

	if (aggressive && player->hasCondition(CONDITION_PACIFIED)) {
		player->sendCancelMessage(RETURNVALUE_YOUAREEXHAUSTED);
		g_game.addMagicEffect(player->getPosition(), CONST_ME_POFF);
		return false;
	}

  if (aggressive && !player->hasFlag(PlayerFlag_IgnoreProtectionZone) && player->getZone() == ZONE_PROTECTION) {
		player->sendCancelMessage(RETURNVALUE_ACTIONNOTPERMITTEDINPROTECTIONZONE);
		return false;
	}

	if (player->hasCondition(CONDITION_MOVEGROUPCOOLDOWN, group) || player->hasCondition(CONDITION_MOVECOOLDOWN, moveId) || (secondaryGroup != MOVEGROUP_NONE && player->hasCondition(CONDITION_MOVEGROUPCOOLDOWN, secondaryGroup))) {
		player->sendCancelMessage(RETURNVALUE_YOUAREEXHAUSTED);

		if (isInstant()) {
			g_game.addMagicEffect(player->getPosition(), CONST_ME_POFF);
		}

		return false;
	}

	if (player->getLevel() < level) {
		player->sendCancelMessage(RETURNVALUE_NOTENOUGHLEVEL);
		g_game.addMagicEffect(player->getPosition(), CONST_ME_POFF);
		return false;
	}

	if (player->getMagicLevel() < magLevel) {
		player->sendCancelMessage(RETURNVALUE_NOTENOUGHMAGICLEVEL);
		g_game.addMagicEffect(player->getPosition(), CONST_ME_POFF);
		return false;
	}

	if (player->getMana() < getManaCost(player) && !player->hasFlag(PlayerFlag_HasInfiniteMana)) {
		player->sendCancelMessage(RETURNVALUE_NOTENOUGHMANA);
		g_game.addMagicEffect(player->getPosition(), CONST_ME_POFF);
		return false;
	}

	if (player->getSoul() < soul && !player->hasFlag(PlayerFlag_HasInfiniteSoul)) {
		player->sendCancelMessage(RETURNVALUE_NOTENOUGHSOUL);
		g_game.addMagicEffect(player->getPosition(), CONST_ME_POFF);
		return false;
	}

	if (isInstant() && isLearnable()) {
		if (!player->hasLearnedInstantMove(getName())) {
			player->sendCancelMessage(RETURNVALUE_YOUNEEDTOLEARNTHISMOVE);
			g_game.addMagicEffect(player->getPosition(), CONST_ME_POFF);
			return false;
		}
	} else if (!vocMoveMap.empty() && vocMoveMap.find(player->getVocationId()) == vocMoveMap.end()) {
		player->sendCancelMessage(RETURNVALUE_YOURVOCATIONCANNOTUSETHISMOVE);
		g_game.addMagicEffect(player->getPosition(), CONST_ME_POFF);
		return false;
	}

	if (needWeapon) {
		switch (player->getWeaponType()) {
			case WEAPON_SWORD:
			case WEAPON_CLUB:
			case WEAPON_AXE:
				break;

			default: {
				player->sendCancelMessage(RETURNVALUE_YOUNEEDAWEAPONTOUSETHISMOVE);
				g_game.addMagicEffect(player->getPosition(), CONST_ME_POFF);
				return false;
			}
		}
	}

	if (isPremium() && !player->isPremium()) {
		player->sendCancelMessage(RETURNVALUE_YOUNEEDPREMIUMACCOUNT);
		g_game.addMagicEffect(player->getPosition(), CONST_ME_POFF);
		return false;
	}

	return true;
}

bool Move::playerInstantMoveCheck(Player* player, const Position& toPos)
{
	if (toPos.x == 0xFFFF) {
		return true;
	}

	const Position& playerPos = player->getPosition();
	if (playerPos.z > toPos.z) {
		player->sendCancelMessage(RETURNVALUE_FIRSTGOUPSTAIRS);
		g_game.addMagicEffect(player->getPosition(), CONST_ME_POFF);
		return false;
	} else if (playerPos.z < toPos.z) {
		player->sendCancelMessage(RETURNVALUE_FIRSTGODOWNSTAIRS);
		g_game.addMagicEffect(player->getPosition(), CONST_ME_POFF);
		return false;
	}

	Tile* tile = g_game.map.getTile(toPos);
	if (!tile) {
		tile = new StaticTile(toPos.x, toPos.y, toPos.z);
		g_game.map.setTile(toPos, tile);
	}

	ReturnValue ret = Combat::canDoCombat(player, tile, aggressive);
	if (ret != RETURNVALUE_NOERROR) {
		player->sendCancelMessage(ret);
		g_game.addMagicEffect(player->getPosition(), CONST_ME_POFF);
		return false;
	}

	if (blockingCreature && tile->getBottomVisibleCreature(player) != nullptr) {
		player->sendCancelMessage(RETURNVALUE_NOTENOUGHROOM);
		g_game.addMagicEffect(player->getPosition(), CONST_ME_POFF);
		return false;
	}

	if (blockingSolid && tile->hasFlag(TILESTATE_BLOCKSOLID)) {
		player->sendCancelMessage(RETURNVALUE_NOTENOUGHROOM);
		g_game.addMagicEffect(player->getPosition(), CONST_ME_POFF);
		return false;
	}

	return true;
}

bool Move::playerRuneMoveCheck(Player* player, const Position& toPos)
{
	if (!playerMoveCheck(player)) {
		return false;
	}

	if (toPos.x == 0xFFFF) {
		return true;
	}

	const Position& playerPos = player->getPosition();
	if (playerPos.z > toPos.z) {
		player->sendCancelMessage(RETURNVALUE_FIRSTGOUPSTAIRS);
		g_game.addMagicEffect(player->getPosition(), CONST_ME_POFF);
		return false;
	} else if (playerPos.z < toPos.z) {
		player->sendCancelMessage(RETURNVALUE_FIRSTGODOWNSTAIRS);
		g_game.addMagicEffect(player->getPosition(), CONST_ME_POFF);
		return false;
	}

	Tile* tile = g_game.map.getTile(toPos);
	if (!tile) {
		player->sendCancelMessage(RETURNVALUE_NOTPOSSIBLE);
		g_game.addMagicEffect(player->getPosition(), CONST_ME_POFF);
		return false;
	}

	if (range != -1 && !g_game.canThrowObjectTo(playerPos, toPos, true, range, range)) {
		player->sendCancelMessage(RETURNVALUE_DESTINATIONOUTOFREACH);
		g_game.addMagicEffect(player->getPosition(), CONST_ME_POFF);
		return false;
	}

	ReturnValue ret = Combat::canDoCombat(player, tile, aggressive);
	if (ret != RETURNVALUE_NOERROR) {
		player->sendCancelMessage(ret);
		g_game.addMagicEffect(player->getPosition(), CONST_ME_POFF);
		return false;
	}

	const Creature* topVisibleCreature = tile->getBottomVisibleCreature(player);
	if (blockingCreature && topVisibleCreature) {
		player->sendCancelMessage(RETURNVALUE_NOTENOUGHROOM);
		g_game.addMagicEffect(player->getPosition(), CONST_ME_POFF);
		return false;
	} else if (blockingSolid && tile->hasFlag(TILESTATE_BLOCKSOLID)) {
		player->sendCancelMessage(RETURNVALUE_NOTENOUGHROOM);
		g_game.addMagicEffect(player->getPosition(), CONST_ME_POFF);
		return false;
	}

	if (needTarget && !topVisibleCreature) {
		player->sendCancelMessage(RETURNVALUE_CANONLYUSETHISRUNEONCREATURES);
		g_game.addMagicEffect(player->getPosition(), CONST_ME_POFF);
		return false;
	}

	if (aggressive && needTarget && topVisibleCreature && player->hasSecureMode()) {
		const Player* targetPlayer = topVisibleCreature->getPlayer();
		if (targetPlayer && targetPlayer != player && player->getSkullClient(targetPlayer) == SKULL_NONE && !Combat::isInPvpZone(player, targetPlayer)) {
			player->sendCancelMessage(RETURNVALUE_TURNSECUREMODETOATTACKUNMARKEDPLAYERS);
			g_game.addMagicEffect(player->getPosition(), CONST_ME_POFF);
			return false;
		}
	}
	return true;
}

void Move::postCastMove(Player* player, bool finishedCast /*= true*/, bool payCost /*= true*/) const
{
	if (finishedCast) {
		if (!player->hasFlag(PlayerFlag_HasNoExhaustion)) {
			if (cooldown > 0) {
				Condition* condition = Condition::createCondition(CONDITIONID_DEFAULT, CONDITION_MOVECOOLDOWN, cooldown, 0, false, moveId);
				player->addCondition(condition);
			}

			if (groupCooldown > 0) {
				Condition* condition = Condition::createCondition(CONDITIONID_DEFAULT, CONDITION_MOVEGROUPCOOLDOWN, groupCooldown, 0, false, group);
				player->addCondition(condition);
			}

			if (secondaryGroupCooldown > 0) {
				Condition* condition = Condition::createCondition(CONDITIONID_DEFAULT, CONDITION_MOVEGROUPCOOLDOWN, secondaryGroupCooldown, 0, false, secondaryGroup);
				player->addCondition(condition);
			}
		}

		if (aggressive) {
			player->addInFightTicks();
		}
	}

	if (payCost) {
		Move::postCastMove(player, getManaCost(player), getSoulCost());
	}
}

void Move::postCastMove(Player* player, uint32_t manaCost, uint32_t soulCost)
{
	if (manaCost > 0) {
		player->addManaSpent(manaCost);
		player->changeMana(-static_cast<int32_t>(manaCost));
	}

	if (!player->hasFlag(PlayerFlag_HasInfiniteSoul)) {
		if (soulCost > 0) {
			player->changeSoul(-static_cast<int32_t>(soulCost));
		}
	}
}

uint32_t Move::getManaCost(const Player* player) const
{
	if (mana != 0) {
		return mana;
	}

	if (manaPercent != 0) {
		uint32_t maxMana = player->getMaxMana();
		uint32_t manaCost = (maxMana * manaPercent) / 100;
		return manaCost;
	}

	return 0;
}

std::string InstantMove::getScriptEventName() const
{
	return "onCastMove";
}

bool InstantMove::configureEvent(const pugi::xml_node& node)
{
	if (!Move::configureMove(node)) {
		return false;
	}

	if (!TalkAction::configureEvent(node)) {
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

bool InstantMove::playerCastInstant(Player* player, std::string& param)
{
	if (!playerMoveCheck(player)) {
		return false;
	}

	LuaVariant var;

	if (selfTarget) {
		var.type = VARIANT_NUMBER;
		var.number = player->getID();
	} else if (needTarget || casterTargetOrDirection) {
		Creature* target = nullptr;
		bool useDirection = false;

		if (hasParam) {
			Player* playerTarget = nullptr;
			ReturnValue ret = g_game.getPlayerByNameWildcard(param, playerTarget);

			if (playerTarget && playerTarget->isAccessPlayer() && !player->isAccessPlayer()) {
				playerTarget = nullptr;
			}

			target = playerTarget;
			if (!target || target->getHealth() <= 0) {
				if (!casterTargetOrDirection) {
					if (cooldown > 0) {
						Condition* condition = Condition::createCondition(CONDITIONID_DEFAULT, CONDITION_MOVECOOLDOWN, cooldown, 0, false, moveId);
						player->addCondition(condition);
					}

					if (groupCooldown > 0) {
						Condition* condition = Condition::createCondition(CONDITIONID_DEFAULT, CONDITION_MOVEGROUPCOOLDOWN, groupCooldown, 0, false, group);
						player->addCondition(condition);
					}

					if (secondaryGroupCooldown > 0) {
						Condition* condition = Condition::createCondition(CONDITIONID_DEFAULT, CONDITION_MOVEGROUPCOOLDOWN, secondaryGroupCooldown, 0, false, secondaryGroup);
						player->addCondition(condition);
					}

					player->sendCancelMessage(ret);
					g_game.addMagicEffect(player->getPosition(), CONST_ME_POFF);
					return false;
				}

				useDirection = true;
			}

			if (playerTarget) {
				param = playerTarget->getName();
			}
		} else {
			target = player->getAttackedCreature();
			if (!target || target->getHealth() <= 0) {
				if (!casterTargetOrDirection) {
					player->sendCancelMessage(RETURNVALUE_YOUCANONLYUSEITONCREATURES);
					g_game.addMagicEffect(player->getPosition(), CONST_ME_POFF);
					return false;
				}

				useDirection = true;
			}
		}

		if (!useDirection) {
			if (!canThrowMove(player, target)) {
				player->sendCancelMessage(RETURNVALUE_CREATUREISNOTREACHABLE);
				g_game.addMagicEffect(player->getPosition(), CONST_ME_POFF);
				return false;
			}

			var.type = VARIANT_NUMBER;
			var.number = target->getID();
		} else {
			var.type = VARIANT_POSITION;
			var.pos = Moves::getCasterPosition(player, player->getDirection());

			if (!playerInstantMoveCheck(player, var.pos)) {
				return false;
			}
		}
	} else if (hasParam) {
		var.type = VARIANT_STRING;

		if (getHasPlayerNameParam()) {
			Player* playerTarget = nullptr;
			ReturnValue ret = g_game.getPlayerByNameWildcard(param, playerTarget);

			if (ret != RETURNVALUE_NOERROR) {
				if (cooldown > 0) {
					Condition* condition = Condition::createCondition(CONDITIONID_DEFAULT, CONDITION_MOVECOOLDOWN, cooldown, 0, false, moveId);
					player->addCondition(condition);
				}

				if (groupCooldown > 0) {
					Condition* condition = Condition::createCondition(CONDITIONID_DEFAULT, CONDITION_MOVEGROUPCOOLDOWN, groupCooldown, 0, false, group);
					player->addCondition(condition);
				}

				if (secondaryGroupCooldown > 0) {
					Condition* condition = Condition::createCondition(CONDITIONID_DEFAULT, CONDITION_MOVEGROUPCOOLDOWN, secondaryGroupCooldown, 0, false, secondaryGroup);
					player->addCondition(condition);
				}

				player->sendCancelMessage(ret);
				g_game.addMagicEffect(player->getPosition(), CONST_ME_POFF);
				return false;
			}

			if (playerTarget && (!playerTarget->isAccessPlayer() || player->isAccessPlayer())) {
				param = playerTarget->getName();
			}
		}

		var.text = param;
	} else {
		var.type = VARIANT_POSITION;

		if (needDirection) {
			var.pos = Moves::getCasterPosition(player, player->getDirection());
		} else {
			var.pos = player->getPosition();
		}

		if (!playerInstantMoveCheck(player, var.pos)) {
			return false;
		}
	}

	bool result = internalCastMove(player, var);
	if (result) {
		postCastMove(player);
	}

	return result;
}

bool InstantMove::canThrowMove(const Creature* creature, const Creature* target) const
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

bool InstantMove::castMove(Creature* creature)
{
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

bool InstantMove::castMove(Creature* creature, Creature* target)
{
	if (needTarget) {
		LuaVariant var;
		var.type = VARIANT_NUMBER;
		var.number = target->getID();
		return internalCastMove(creature, var);
	} else {
		return castMove(creature);
	}
}

bool InstantMove::internalCastMove(Creature* creature, const LuaVariant& var)
{
	return executeCastMove(creature, var);
}

bool InstantMove::executeCastMove(Creature* creature, const LuaVariant& var)
{
	//onCastMove(creature, var)
	if (!scriptInterface->reserveScriptEnv()) {
		std::cout << "[Error - InstantMove::executeCastMove] Call stack overflow" << std::endl;
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

bool InstantMove::canCast(const Player* player) const
{
	if (player->hasFlag(PlayerFlag_CannotUseMoves)) {
		return false;
	}

	if (player->hasFlag(PlayerFlag_IgnoreMoveCheck)) {
		return true;
	}

	if (isLearnable()) {
		if (player->hasLearnedInstantMove(getName())) {
			return true;
		}
	} else {
		if (vocMoveMap.empty() || vocMoveMap.find(player->getVocationId()) != vocMoveMap.end()) {
			return true;
		}
	}

	return false;
}

std::string RuneMove::getScriptEventName() const
{
	return "onCastMove";
}

bool RuneMove::configureEvent(const pugi::xml_node& node)
{
	if (!Move::configureMove(node)) {
		return false;
	}

	if (!Action::configureEvent(node)) {
		return false;
	}

	pugi::xml_attribute attr;
	if (!(attr = node.attribute("id"))) {
		std::cout << "[Error - RuneMove::configureMove] Rune move without id." << std::endl;
		return false;
	}
	runeId = pugi::cast<uint16_t>(attr.value());

	uint32_t charges;
	if ((attr = node.attribute("charges"))) {
		charges = pugi::cast<uint32_t>(attr.value());
	} else {
		charges = 0;
	}

	hasCharges = (charges > 0);
	if (magLevel != 0 || level != 0) {
		//Change information in the ItemType to get accurate description
		ItemType& iType = Item::items.getItemType(runeId);
		iType.runeMagLevel = magLevel;
		iType.runeLevel = level;
		iType.charges = charges;
	}

	return true;
}

ReturnValue RuneMove::canExecuteAction(const Player* player, const Position& toPos)
{
	if (player->hasFlag(PlayerFlag_CannotUseMoves)) {
		return RETURNVALUE_CANNOTUSETHISOBJECT;
	}

	ReturnValue ret = Action::canExecuteAction(player, toPos);
	if (ret != RETURNVALUE_NOERROR) {
		return ret;
	}

	if (toPos.x == 0xFFFF) {
		if (needTarget) {
			return RETURNVALUE_CANONLYUSETHISRUNEONCREATURES;
		} else if (!selfTarget) {
			return RETURNVALUE_NOTENOUGHROOM;
		}
	}

	return RETURNVALUE_NOERROR;
}

bool RuneMove::executeUse(Player* player, Item* item, const Position&, Thing* target, const Position& toPosition, bool isHotkey)
{
	if (!playerRuneMoveCheck(player, toPosition)) {
		return false;
	}

	if (!scripted) {
		return false;
	}

	LuaVariant var;

	if (needTarget) {
		var.type = VARIANT_NUMBER;

		if (target == nullptr) {
			Tile* toTile = g_game.map.getTile(toPosition);
			if (toTile) {
				const Creature* visibleCreature = toTile->getBottomVisibleCreature(player);
				if (visibleCreature) {
					var.number = visibleCreature->getID();
				}
			}
		} else {
			var.number = target->getCreature()->getID();
		}
	} else {
		var.type = VARIANT_POSITION;
		var.pos = toPosition;
	}

	if (!internalCastMove(player, var, isHotkey)) {
		return false;
	}

	postCastMove(player);
	if (hasCharges && item && g_config.getBoolean(ConfigManager::REMOVE_RUNE_CHARGES)) {
		int32_t newCount = std::max<int32_t>(0, item->getItemCount() - 1);
		g_game.transformItem(item, item->getID(), newCount);
	}
	return true;
}

bool RuneMove::castMove(Creature* creature)
{
	LuaVariant var;
	var.type = VARIANT_NUMBER;
	var.number = creature->getID();
	return internalCastMove(creature, var, false);
}

bool RuneMove::castMove(Creature* creature, Creature* target)
{
	LuaVariant var;
	var.type = VARIANT_NUMBER;
	var.number = target->getID();
	return internalCastMove(creature, var, false);
}

bool RuneMove::internalCastMove(Creature* creature, const LuaVariant& var, bool isHotkey)
{
	bool result;
	if (scripted) {
		result = executeCastMove(creature, var, isHotkey);
	} else {
		result = false;
	}
	return result;
}

bool RuneMove::executeCastMove(Creature* creature, const LuaVariant& var, bool isHotkey)
{
	//onCastMove(creature, var, isHotkey)
	if (!scriptInterface->reserveScriptEnv()) {
		std::cout << "[Error - RuneMove::executeCastMove] Call stack overflow" << std::endl;
		return false;
	}

	ScriptEnvironment* env = scriptInterface->getScriptEnv();
	env->setScriptId(scriptId, scriptInterface);

	lua_State* L = scriptInterface->getLuaState();

	scriptInterface->pushFunction(scriptId);

	LuaScriptInterface::pushUserdata<Creature>(L, creature);
	LuaScriptInterface::setCreatureMetatable(L, -1, creature);

	LuaScriptInterface::pushVariant(L, var);

	LuaScriptInterface::pushBoolean(L, isHotkey);

	return scriptInterface->callFunction(3);
}
