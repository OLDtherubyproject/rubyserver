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

#include "pokemon.h"
#include "game.h"
#include "moves.h"
#include "foods.h"
#include "events.h"

extern Game g_game;
extern Pokemons g_pokemons;
extern Moves* g_moves;
extern Events* g_events;

int32_t Pokemon::despawnRange;
int32_t Pokemon::despawnRadius;

uint32_t Pokemon::pokemonAutoID = 0x40000000;

Pokemon* Pokemon::createPokemon(const std::string& name, int32_t plevel /* = 1 */, bool spawn /* = true */)
{
	PokemonType* mType = g_pokemons.getPokemonType(name);
	if (!mType) {
		return nullptr;
	}
	return new Pokemon(mType, plevel, spawn);
}

Pokemon::Pokemon(PokemonType* mType, int32_t plevel /* = 1 */, bool spawn /* = true */) :
	Creature(),
	strDescription(mType->nameDescription),
	mType(mType)
{
	if (spawn) {
		setDittoStatus(boolean_random(mType->info.dittoChance / 100) == 1);
		setShinyStatus(boolean_random(mType->info.shiny.chance / 100) == 1);
		defaultOutfit = (getShinyStatus() ? mType->info.shiny.outfit : mType->info.outfit);
		currentOutfit = (getShinyStatus() ? mType->info.shiny.outfit : mType->info.outfit);
		gender = mType->info.gender;
		internalLight = mType->info.light;
		hiddenHealth = mType->info.hiddenHealth;
		name = mType->name;
		ivs.hp = uniform_random(1, 31);
		ivs.attack = uniform_random(1, 31);
		ivs.defense = uniform_random(1, 31);
		ivs.speed = uniform_random(1, 31);
		ivs.special_attack = uniform_random(1, 31);
		ivs.special_defense = uniform_random(1, 31);

		// random moves
		for (const auto& move : mType->info.moves) {
			if (normal_random(1, 100) <= std::get<0>(move.second)) {
				addMove(std::pair<uint16_t, uint16_t>(move.first, std::get<1>(move.second)));
			}
		}

		randomGender();
	}

	level = plevel;
	health = getMaxHealth();

	if (getNumber() == 132 && !getDittoStatus()) {
		setDittoStatus(true);
	}

	// register creature events
	for (const std::string& scriptName : mType->info.scripts) {
		if (!registerCreatureEvent(scriptName)) {
			std::cout << "[Warning - Pokemon::Pokemon] Unknown event name: " << scriptName << std::endl;
		}
	}
}

Pokemon::Pokemon()
{
	//
}

Pokemon::~Pokemon()
{
	clearTargetList();
	clearFriendList();
}

void Pokemon::addList()
{
	g_game.addPokemon(this);
}

void Pokemon::removeList()
{
	g_game.removePokemon(this);
}

bool Pokemon::canSee(const Position& pos) const
{
	return Creature::canSee(getPosition(), pos, 9, 9);
}

bool Pokemon::canWalkOnFieldType(CombatType_t combatType) const
{
	switch (combatType) {
		case COMBAT_ELECTRICDAMAGE:
			return mType->info.canWalkOnEnergy;
		case COMBAT_FIREDAMAGE:
			return mType->info.canWalkOnFire;
		case COMBAT_GRASSDAMAGE:
			return mType->info.canWalkOnPoison;
		default:
			return true;
	}
}

void Pokemon::onAttackedCreatureDisappear(bool)
{
	attackTicks = 0;
	extraMeleeAttack = true;
}

void Pokemon::onCreatureAppear(Creature* creature, bool isLogin)
{
	Creature::onCreatureAppear(creature, isLogin);

	if (mType->info.creatureAppearEvent != -1) {
		// onCreatureAppear(self, creature)
		LuaScriptInterface* scriptInterface = mType->info.scriptInterface;
		if (!scriptInterface->reserveScriptEnv()) {
			std::cout << "[Error - Pokemon::onCreatureAppear] Call stack overflow" << std::endl;
			return;
		}

		ScriptEnvironment* env = scriptInterface->getScriptEnv();
		env->setScriptId(mType->info.creatureAppearEvent, scriptInterface);

		lua_State* L = scriptInterface->getLuaState();
		scriptInterface->pushFunction(mType->info.creatureAppearEvent);

		LuaScriptInterface::pushUserdata<Pokemon>(L, this);
		LuaScriptInterface::setMetatable(L, -1, "Pokemon");

		LuaScriptInterface::pushUserdata<Creature>(L, creature);
		LuaScriptInterface::setCreatureMetatable(L, -1, creature);

		if (scriptInterface->callFunction(2)) {
			return;
		}
	}

	if (creature == this) {
		for (Condition* condition : storedConditionList) {
			addCondition(condition);
		}
		storedConditionList.clear();

		//We just spawned lets look around to see who is there.
		if (isSummon()) {
			isMasterInRange = canSee(getMaster()->getPosition());
		}

		updateTargetList();
		updateIdleStatus();
	} else {
		onCreatureEnter(creature);
	}
}

void Pokemon::onRemoveCreature(Creature* creature, bool isLogout)
{
	Creature::onRemoveCreature(creature, isLogout);

	if (mType->info.creatureDisappearEvent != -1) {
		// onCreatureDisappear(self, creature)
		LuaScriptInterface* scriptInterface = mType->info.scriptInterface;
		if (!scriptInterface->reserveScriptEnv()) {
			std::cout << "[Error - Pokemon::onCreatureDisappear] Call stack overflow" << std::endl;
			return;
		}

		ScriptEnvironment* env = scriptInterface->getScriptEnv();
		env->setScriptId(mType->info.creatureDisappearEvent, scriptInterface);

		lua_State* L = scriptInterface->getLuaState();
		scriptInterface->pushFunction(mType->info.creatureDisappearEvent);

		LuaScriptInterface::pushUserdata<Pokemon>(L, this);
		LuaScriptInterface::setMetatable(L, -1, "Pokemon");

		LuaScriptInterface::pushUserdata<Creature>(L, creature);
		LuaScriptInterface::setCreatureMetatable(L, -1, creature);

		if (scriptInterface->callFunction(2)) {
			return;
		}
	}

	if (creature == this) {
		if (spawn) {
			spawn->startSpawnCheck();
		}

		setIdle(true);
	} else {
		onCreatureLeave(creature);
	}
}

void Pokemon::onCreatureMove(Creature* creature, const Tile* newTile, const Position& newPos,
                             const Tile* oldTile, const Position& oldPos, bool teleport)
{
	Creature::onCreatureMove(creature, newTile, newPos, oldTile, oldPos, teleport);

	if (mType->info.creatureMoveEvent != -1) {
		// onCreatureMove(self, creature, oldPosition, newPosition)
		LuaScriptInterface* scriptInterface = mType->info.scriptInterface;
		if (!scriptInterface->reserveScriptEnv()) {
			std::cout << "[Error - Pokemon::onCreatureMove] Call stack overflow" << std::endl;
			return;
		}

		ScriptEnvironment* env = scriptInterface->getScriptEnv();
		env->setScriptId(mType->info.creatureMoveEvent, scriptInterface);

		lua_State* L = scriptInterface->getLuaState();
		scriptInterface->pushFunction(mType->info.creatureMoveEvent);

		LuaScriptInterface::pushUserdata<Pokemon>(L, this);
		LuaScriptInterface::setMetatable(L, -1, "Pokemon");

		LuaScriptInterface::pushUserdata<Creature>(L, creature);
		LuaScriptInterface::setCreatureMetatable(L, -1, creature);

		LuaScriptInterface::pushPosition(L, oldPos);
		LuaScriptInterface::pushPosition(L, newPos);

		if (scriptInterface->callFunction(4)) {
			return;
		}
	}

	if (creature == this) {
		if (isSummon()) {
			isMasterInRange = canSee(getMaster()->getPosition());
		}

		updateTargetList();
		updateIdleStatus();
	} else {
		bool canSeeNewPos = canSee(newPos);
		bool canSeeOldPos = canSee(oldPos);

		if (canSeeNewPos && !canSeeOldPos) {
			onCreatureEnter(creature);
		} else if (!canSeeNewPos && canSeeOldPos) {
			onCreatureLeave(creature);
		}

		if (canSeeNewPos && isSummon() && getMaster() == creature) {
			isMasterInRange = true;    //Follow master again
		}

		updateIdleStatus();

		if (!isSummon()) {
			if (followCreature) {
				const Position& followPosition = followCreature->getPosition();
				const Position& position = getPosition();

				int32_t offset_x = Position::getDistanceX(followPosition, position);
				int32_t offset_y = Position::getDistanceY(followPosition, position);
				if ((offset_x > 1 || offset_y > 1) && mType->info.changeTargetChance > 0) {
					Direction dir = getDirectionTo(position, followPosition);
					const Position& checkPosition = getNextPosition(dir, position);

					Tile* tile = g_game.map.getTile(checkPosition);
					if (tile) {
						Creature* topCreature = tile->getTopCreature();
						if (topCreature && followCreature != topCreature && isOpponent(topCreature)) {
							selectTarget(topCreature);
						}
					}
				}
			} else if (isOpponent(creature)) {
				//we have no target lets try pick this one
				selectTarget(creature);
			}
		}
	}
}

void Pokemon::onCreatureSay(Creature* creature, SpeakClasses type, const std::string& text)
{
	Creature::onCreatureSay(creature, type, text);

	if (mType->info.creatureSayEvent != -1) {
		// onCreatureSay(self, creature, type, message)
		LuaScriptInterface* scriptInterface = mType->info.scriptInterface;
		if (!scriptInterface->reserveScriptEnv()) {
			std::cout << "[Error - Pokemon::onCreatureSay] Call stack overflow" << std::endl;
			return;
		}

		ScriptEnvironment* env = scriptInterface->getScriptEnv();
		env->setScriptId(mType->info.creatureSayEvent, scriptInterface);

		lua_State* L = scriptInterface->getLuaState();
		scriptInterface->pushFunction(mType->info.creatureSayEvent);

		LuaScriptInterface::pushUserdata<Pokemon>(L, this);
		LuaScriptInterface::setMetatable(L, -1, "Pokemon");

		LuaScriptInterface::pushUserdata<Creature>(L, creature);
		LuaScriptInterface::setCreatureMetatable(L, -1, creature);

		lua_pushnumber(L, type);
		LuaScriptInterface::pushString(L, text);

		scriptInterface->callVoidFunction(4);
	}
}

void Pokemon::addFriend(Creature* creature)
{
	assert(creature != this);
	auto result = friendList.insert(creature);
	if (result.second) {
		creature->incrementReferenceCounter();
	}
}

void Pokemon::removeFriend(Creature* creature)
{
	auto it = friendList.find(creature);
	if (it != friendList.end()) {
		creature->decrementReferenceCounter();
		friendList.erase(it);
	}
}

void Pokemon::addTarget(Creature* creature, bool pushFront/* = false*/)
{
	assert(creature != this);
	if (std::find(targetList.begin(), targetList.end(), creature) == targetList.end()) {
		creature->incrementReferenceCounter();
		if (pushFront) {
			targetList.push_front(creature);
		} else {
			targetList.push_back(creature);
		}
	}
}

void Pokemon::removeTarget(Creature* creature)
{
	auto it = std::find(targetList.begin(), targetList.end(), creature);
	if (it != targetList.end()) {
		creature->decrementReferenceCounter();
		targetList.erase(it);
	}
}

void Pokemon::updateTargetList()
{
	auto friendIterator = friendList.begin();
	while (friendIterator != friendList.end()) {
		Creature* creature = *friendIterator;
		if (creature->getHealth() <= 0 || !canSee(creature->getPosition())) {
			creature->decrementReferenceCounter();
			friendIterator = friendList.erase(friendIterator);
		} else {
			++friendIterator;
		}
	}

	auto targetIterator = targetList.begin();
	while (targetIterator != targetList.end()) {
		Creature* creature = *targetIterator;
		if (creature->getHealth() <= 0 || !canSee(creature->getPosition())) {
			creature->decrementReferenceCounter();
			targetIterator = targetList.erase(targetIterator);
		} else {
			++targetIterator;
		}
	}

	SpectatorHashSet spectators;
	g_game.map.getSpectators(spectators, position, true);
	spectators.erase(this);
	for (Creature* spectator : spectators) {
		if (canSee(spectator->getPosition())) {
			onCreatureFound(spectator);
		}
	}
}

void Pokemon::clearTargetList()
{
	for (Creature* creature : targetList) {
		creature->decrementReferenceCounter();
	}
	targetList.clear();
}

void Pokemon::clearFriendList()
{
	for (Creature* creature : friendList) {
		creature->decrementReferenceCounter();
	}
	friendList.clear();
}

void Pokemon::onCreatureFound(Creature* creature, bool pushFront/* = false*/)
{
	if (isFriend(creature)) {
		addFriend(creature);
	}

	if (isOpponent(creature)) {
		addTarget(creature, pushFront);
	}

	updateIdleStatus();
}

void Pokemon::onCreatureEnter(Creature* creature)
{
	// std::cout << "onCreatureEnter - " << creature->getName() << std::endl;

	if (getMaster() == creature) {
		//Follow master again
		isMasterInRange = true;
	}

	onCreatureFound(creature, true);
}

bool Pokemon::isFriend(const Creature* creature) const
{
	if (isSummon() && getMaster()->getPlayer()) {
		const Player* masterPlayer = getMaster()->getPlayer();
		const Player* tmpPlayer = nullptr;

		if (creature->getPlayer()) {
			tmpPlayer = creature->getPlayer();
		} else {
			const Creature* creatureMaster = creature->getMaster();

			if (creatureMaster && creatureMaster->getPlayer()) {
				tmpPlayer = creatureMaster->getPlayer();
			}
		}

		if (tmpPlayer && (tmpPlayer == getMaster() || masterPlayer->isPartner(tmpPlayer))) {
			return true;
		}
	} else if (creature->getPokemon() && !creature->isSummon()) {
		return true;
	}

	return false;
}

bool Pokemon::isOpponent(const Creature* creature) const
{
	if (isSummon() && getMaster()->getPlayer()) {
		if (creature != getMaster()) {
			return true;
		}
	} else {
		if ((creature->getPlayer() && !creature->getPlayer()->hasFlag(PlayerFlag_IgnoredByPokemons)) ||
		        (creature->getMaster() && creature->getMaster()->getPlayer())) {
			return true;
		}
	}

	return false;
}

void Pokemon::onCreatureLeave(Creature* creature)
{
	// std::cout << "onCreatureLeave - " << creature->getName() << std::endl;

	if (getMaster() == creature) {
		//Take random steps and only use defense abilities (e.g. heal) until its master comes back
		isMasterInRange = false;
	}

	//update friendList
	if (isFriend(creature)) {
		removeFriend(creature);
	}

	//update targetList
	if (isOpponent(creature)) {
		removeTarget(creature);
		if (targetList.empty()) {
			updateIdleStatus();
		}
	}
}

bool Pokemon::searchTarget(TargetSearchType_t searchType /*= TARGETSEARCH_DEFAULT*/)
{
	std::list<Creature*> resultList;
	const Position& myPos = getPosition();

	for (Creature* creature : targetList) {
		if (followCreature != creature && isTarget(creature)) {
			if (searchType == TARGETSEARCH_RANDOM || canUseAttack(myPos, creature)) {
				resultList.push_back(creature);
			}
		}
	}

	switch (searchType) {
		case TARGETSEARCH_NEAREST: {
			Creature* target = nullptr;
			if (!resultList.empty()) {
				auto it = resultList.begin();
				target = *it;

				if (++it != resultList.end()) {
					const Position& targetPosition = target->getPosition();
					int32_t minRange = Position::getDistanceX(myPos, targetPosition) + Position::getDistanceY(myPos, targetPosition);
					do {
						const Position& pos = (*it)->getPosition();

						int32_t distance = Position::getDistanceX(myPos, pos) + Position::getDistanceY(myPos, pos);
						if (distance < minRange) {
							target = *it;
							minRange = distance;
						}
					} while (++it != resultList.end());
				}
			} else {
				int32_t minRange = std::numeric_limits<int32_t>::max();
				for (Creature* creature : targetList) {
					if (!isTarget(creature)) {
						continue;
					}

					const Position& pos = creature->getPosition();
					int32_t distance = Position::getDistanceX(myPos, pos) + Position::getDistanceY(myPos, pos);
					if (distance < minRange) {
						target = creature;
						minRange = distance;
					}
				}
			}

			if (target && selectTarget(target)) {
				return true;
			}
			break;
		}

		case TARGETSEARCH_DEFAULT:
		case TARGETSEARCH_ATTACKRANGE:
		case TARGETSEARCH_RANDOM:
		default: {
			if (!resultList.empty()) {
				auto it = resultList.begin();
				std::advance(it, uniform_random(0, resultList.size() - 1));
				return selectTarget(*it);
			}

			if (searchType == TARGETSEARCH_ATTACKRANGE) {
				return false;
			}

			break;
		}
	}

	//lets just pick the first target in the list
	for (Creature* target : targetList) {
		if (followCreature != target && selectTarget(target)) {
			return true;
		}
	}
	return false;
}

void Pokemon::onFollowCreatureComplete(const Creature* creature)
{
	if (creature) {
		auto it = std::find(targetList.begin(), targetList.end(), creature);
		if (it != targetList.end()) {
			Creature* target = (*it);
			targetList.erase(it);

			if (hasFollowPath) {
				targetList.push_front(target);
			} else if (!isSummon()) {
				targetList.push_back(target);
			} else {
				target->decrementReferenceCounter();
			}
		}
	}
}

BlockType_t Pokemon::blockHit(Creature* attacker, CombatType_t combatType, int32_t& damage,
                              bool /* field = false */)
{
	BlockType_t blockType = Creature::blockHit(attacker, combatType, damage);

	if (damage != 0) {
		if (isSuperEffective(combatType)) {
			damage *= 2;
		} else if (isNotVeryEffective(combatType)) {
			damage *= 0.5;
		}
	}

	return blockType;
}

int32_t Pokemon::getMaxHealth() const
{
	int32_t level = getLevel();
	int32_t hp = (((2 * mType->info.baseStats.hp + ivs.hp + ivs.hp + (evs.hp / 30)) * level) / 100) + level + 10;

	return std::max<int32_t>(1, hp);
}

float Pokemon::getAttack() const
{
	float level = (float) getLevel();
	float attack = (((2.0 * mType->info.baseStats.attack + ivs.attack + (evs.attack / 30.0) * level) / 100.0f) + 5.0f) * getAttackMultiplier();

	return attack;
}

float Pokemon::getDefense() const
{
	float level = (float) getLevel();
	float defense = (((2.0 * mType->info.baseStats.defense + ivs.defense + (evs.defense / 30.0) * level) / 100.0f) + 5.0f) * getDefenseMultiplier();

	return defense;
}

float Pokemon::getSpecialAttack() const
{
	float level = (float) getLevel();
	float special_attack = (((2.0 * mType->info.baseStats.special_attack + ivs.special_attack + (evs.special_attack / 30.0) * level) / 100.0f) + 5.0f) * getSpecialAttackMultiplier();
	
	return special_attack;
}

float Pokemon::getSpecialDefense() const
{
	float level = (float) getLevel();
	float special_defense = (((2.0 * mType->info.baseStats.special_defense + ivs.special_defense + (evs.special_defense / 30.0) * level) / 100.0f) + 5.0f) * getSpecialDefenseMultiplier();

	return special_defense;
}

bool Pokemon::castMove(uint16_t moveId, bool ignoreMessages /* = false */)
{	
	Move* move = g_moves->getMove(moveId);

	if (!move) {
		return false;
	}

	Player* player = nullptr;

	if (belongsToPlayer()) {
		player = getMaster()->getPlayer();
		
		if (!player->canCastMove()) {
			return false;
		}

		player->setNextCastMove(OTSYS_TIME() + 1000);
	}

	if (player && (move->getLevel() > player->getLevel())) {
		player->sendTextMessage(MESSAGE_STATUS_CONSOLE_BLUE, "You need level " + std::to_string(move->getLevel()) + " to cast " + move->getName());
		return false;
	}

	if (hasCondition(CONDITION_SLEEP) && !move->ignoreSleep) {
		if (player) {
			player->sendTextMessage(MESSAGE_STATUS_CONSOLE_BLUE, "Your Pokemon can not use any move.");
		}
		return false;
	}

	if (hasCondition(CONDITION_MOVECOOLDOWN, move->getId())) {
		Condition* condition = getCondition(CONDITION_MOVECOOLDOWN, CONDITIONID_DEFAULT, move->getId());
		if (player) {
			uint32_t cooldown = (condition->getEndTime() - OTSYS_TIME()) / 1000;
			std::ostringstream ss;
			ss << "You Have to wait " << cooldown << " second" << (cooldown != 1 ? "s" : "") << " to cast " << move->getName() << " again."; 
			player->sendTextMessage(MESSAGE_STATUS_CONSOLE_BLUE, ss.str());
		}
		return false;
	}

	if (!move->needTarget) {
		if (!move->castMove(this)) {
			g_game.addAnimatedText(getPosition(), 215, "MISS");
		}
	} else {
		Creature* target = getAttackedCreature();

		if (!target || !(target->getHealth() > 0)) {
			if (player) {
				player->sendTextMessage(MESSAGE_STATUS_CONSOLE_BLUE, "Select a target.");
			}
			
			return false;
		}

		if (!move->canThrowMove(this, target)) {
			if (player) {
				player->sendTextMessage(MESSAGE_STATUS_CONSOLE_BLUE, "Please, get closer to cast this move.");
			}

			return false;
		}

		if (!move->castMove(this, target)) {
			g_game.addAnimatedText(getPosition(), 215, "MISS");
		}
	}

	if (move->getCooldown() > 0) {
		uint32_t cooldown = move->getCooldown() - (move->getCooldown() * std::min((getSpeed() / 1000.0), 0.8));
		addCondition(Condition::createCondition(CONDITIONID_DEFAULT, CONDITION_MOVECOOLDOWN, cooldown, 0, false, move->getId()));
	}

	if (!ignoreMessages) {
		g_game.internalCreatureSay(this, TALKTYPE_POKEMON_SAY, move->getName(), false);
		if (player) {
			g_game.internalCreatureSay(player, TALKTYPE_POKEMON_SAY, getName() + ", " + move->getName() + "!", false);
		}
	}

	if (move->isAggressive() && player) {
		player->addInFightTicks();
	}

	if (player) {
		player->setNextCastMove(0);
	}

	return true;
}

bool Pokemon::feed(const FoodType* foodType) {
	Condition* condition = getCondition(CONDITION_REGENERATION, CONDITIONID_DEFAULT);
	if (condition) {
		if ((condition->getTicks() / 1000 + (foodType->getRegen() * 12)) > 1200) {
			if (!belongsToPlayer()) {
				return false;
			}

			getMaster()->getPlayer()->sendTextMessage(MESSAGE_STATUS_SMALL, "Your Pokemon is full.");
			return false;
		}
		
		condition->setTicks(condition->getTicks() + (foodType->getRegen() * 12 * 1000));
		condition->setParam(CONDITION_PARAM_HEALTHGAIN, 5);
		condition->setParam(CONDITION_PARAM_HEALTHTICKS, 10 * 1000);
	} else {
		condition = Condition::createCondition(CONDITIONID_DEFAULT, CONDITION_REGENERATION, foodType->getRegen() * 12 * 1000, 0);
		condition->setTicks(foodType->getRegen() * 12 * 1000);
		condition->setParam(CONDITION_PARAM_HEALTHGAIN, 5);
		condition->setParam(CONDITION_PARAM_HEALTHTICKS, 10 * 1000);
		addCondition(condition);
	}

	g_game.addEffect(getPosition(), CONST_ME_EMOT_SOHAPPY);
	g_game.internalCreatureSay(this, TALKTYPE_POKEMON_SAY, foodType->getSound(), false);
	return true;
}

bool Pokemon::isTarget(const Creature* creature) const
{
	if (creature->isRemoved() || !creature->isAttackable() ||
	        creature->getZone() == ZONE_PROTECTION || !canSeeCreature(creature) ||
			(creature->getPokemon() && creature->getPokemon()->belongsToPlayer() 
			&& creature->getMaster()->getZone() == ZONE_PROTECTION)) {
		return false;
	}

	if (creature->getPosition().z != getPosition().z) {
		return false;
	}
	return true;
}

bool Pokemon::selectTarget(Creature* creature)
{
	if (!isTarget(creature)) {
		return false;
	}

	auto it = std::find(targetList.begin(), targetList.end(), creature);
	if (it == targetList.end()) {
		//Target not found in our target list.
		return false;
	}

	if (isHostile() || isSummon()) {
		if (setAttackedCreature(creature) && !isSummon()) {
			g_dispatcher.addTask(createTask(std::bind(&Game::checkCreatureAttack, &g_game, getID())));
		}
	}
	return setFollowCreature(creature);
}

void Pokemon::setIdle(bool idle)
{
	if (isRemoved() || getHealth() <= 0) {
		return;
	}

	isIdle = idle;

	if (!isIdle) {
		g_game.addCreatureCheck(this);
	} else {
		onIdleStatus();
		clearTargetList();
		clearFriendList();
		Game::removeCreatureCheck(this);
	}
}

void Pokemon::updateIdleStatus()
{
	bool idle = false;

	if (conditions.empty()) {
		if (!isSummon() && targetList.empty()) {
			idle = true;
		}
	}

	setIdle(idle);
}

void Pokemon::onAddCondition(ConditionType_t type)
{
	if (type == CONDITION_FIRE || type == CONDITION_ENERGY || type == CONDITION_POISON) {
		updateMapCache();
	}

	updateIdleStatus();
}

void Pokemon::onEndCondition(ConditionType_t type)
{
	if (type == CONDITION_FIRE || type == CONDITION_ENERGY || type == CONDITION_POISON) {
		ignoreFieldDamage = false;
		updateMapCache();
	}

	updateIdleStatus();
}

void Pokemon::onAddCombatCondition(ConditionType_t type)
{
	if (belongsToPlayer()) {
		switch (type) {
			case CONDITION_POISON:
				getMaster()->getPlayer()->sendTextMessage(MESSAGE_STATUS_DEFAULT, "Your pokemon is poisoned.");
				break;

			case CONDITION_DROWN:
				getMaster()->getPlayer()->sendTextMessage(MESSAGE_STATUS_DEFAULT, "Your pokemon is drowning.");
				break;

			case CONDITION_PARALYZE:
				getMaster()->getPlayer()->sendTextMessage(MESSAGE_STATUS_DEFAULT, "Your pokemon is paralyzed.");
				break;

			case CONDITION_CONFUSION:
				getMaster()->getPlayer()->sendTextMessage(MESSAGE_STATUS_DEFAULT, "Your pokemon is confused.");
				break;

			case CONDITION_CURSED:
				getMaster()->getPlayer()->sendTextMessage(MESSAGE_STATUS_DEFAULT, "Your pokemon is cursed.");
				break;

			case CONDITION_FREEZING:
				getMaster()->getPlayer()->sendTextMessage(MESSAGE_STATUS_DEFAULT, "Your pokemon is freezing.");
				break;

			case CONDITION_DAZZLED:
				getMaster()->getPlayer()->sendTextMessage(MESSAGE_STATUS_DEFAULT, "Your pokemon is dazzled.");
				break;

			case CONDITION_BLEEDING:
				getMaster()->getPlayer()->sendTextMessage(MESSAGE_STATUS_DEFAULT, "Your pokemon is bleeding.");
				break;

			case CONDITION_SLEEP:
				getMaster()->getPlayer()->sendTextMessage(MESSAGE_STATUS_DEFAULT, "Your pokemon is sleeping.");
				break;

			default:
				break;
		}
	}
}

void Pokemon::onCombatRemoveCondition(Condition* condition)
{
	Creature::onCombatRemoveCondition(condition);
}

void Pokemon::onThink(uint32_t interval)
{
	Creature::onThink(interval);

	if (mType->info.thinkEvent != -1) {
		// onThink(self, interval)
		LuaScriptInterface* scriptInterface = mType->info.scriptInterface;
		if (!scriptInterface->reserveScriptEnv()) {
			std::cout << "[Error - Pokemon::onThink] Call stack overflow" << std::endl;
			return;
		}

		ScriptEnvironment* env = scriptInterface->getScriptEnv();
		env->setScriptId(mType->info.thinkEvent, scriptInterface);

		lua_State* L = scriptInterface->getLuaState();
		scriptInterface->pushFunction(mType->info.thinkEvent);

		LuaScriptInterface::pushUserdata<Pokemon>(L, this);
		LuaScriptInterface::setMetatable(L, -1, "Pokemon");

		lua_pushnumber(L, interval);

		if (scriptInterface->callFunction(2)) {
			return;
		}
	}

	if (!isInSpawnRange(position)) {
		g_game.internalTeleport(this, masterPos);
		setIdle(true);
	} else {
		updateIdleStatus();

		if (!isIdle) {
			addEventWalk();

			if (isSummon()) {
				if(needTeleportToPlayer && teleportToPlayer())
					needTeleportToPlayer = false;

				if (!attackedCreature) {
					if (getMaster() && getMaster()->getAttackedCreature()) {
						//This happens if the pokemon is summoned during combat
						selectTarget(getMaster()->getAttackedCreature());
					} else if (getMaster() != followCreature) {
						//Our master has not ordered us to attack anything, lets follow him around instead.
						if (followMaster)
							setFollowCreature(getMaster());
					}
				} else if (attackedCreature == this) {
					setFollowCreature(nullptr);
				} else if (followCreature != attackedCreature) {
					//This happens just after a master orders an attack, so lets follow it aswell.
					setFollowCreature(attackedCreature);
				}
			} else {
				if (checkSpawn()) {
					return;
				}

				if (!targetList.empty()) {
					if (!followCreature || !hasFollowPath) {
						searchTarget();
					} else if (isFleeing()) {
						if (attackedCreature && !canUseAttack(getPosition(), attackedCreature)) {
							searchTarget(TARGETSEARCH_ATTACKRANGE);
						}
					}
				}
			}

			onThinkTarget(interval);
			onThinkYell(interval);
			onThinkDefense(interval);
			onThinkEmoticon(interval);
		}
	}
}

void Pokemon::doAttacking(uint32_t interval)
{
	//std::unique_ptr<Combat*> combat;

	if (!attackedCreature || (isSummon() && attackedCreature == this)) {
		return;
	}

	if (!isSummon()) {
		for (const auto& move : moves) {
			if (normal_random(1, 100) <= move.second) {
				castMove(move.first);
			}
		}
	}

	updateLookDirection();
}

bool Pokemon::canUseAttack(const Position& pos, const Creature* target) const
{
	if (isHostile()) {
		return g_game.isSightClear(pos, target->getPosition(), true);
	}
	return true;
}

void Pokemon::onThinkTarget(uint32_t interval)
{
	if (!isSummon()) {
		if (attackedCreature) {
			if (Player* player = attackedCreature->getPlayer()) {
				if (player->getHisPokemon()) {
					selectTarget(player->getHisPokemon());
				}
			} else if (Pokemon* pokemon = attackedCreature->getPokemon()) {
				if (pokemon->belongsToPlayer() && pokemon->getMaster()->getZone() == ZONE_PROTECTION) {
					removeTarget(attackedCreature);
					setAttackedCreature(nullptr);
					setFollowCreature(nullptr);
					if (targetList.empty()) {
						updateIdleStatus();
					}
				}
			}
		}

		if (mType->info.changeTargetSpeed != 0) {
			bool canChangeTarget = true;

			if (targetChangeCooldown > 0) {
				targetChangeCooldown -= interval;

				if (targetChangeCooldown <= 0) {
					targetChangeCooldown = 0;
					targetChangeTicks = mType->info.changeTargetSpeed;
				} else {
					canChangeTarget = false;
				}
			}

			if (canChangeTarget) {
				targetChangeTicks += interval;

				if (targetChangeTicks >= mType->info.changeTargetSpeed) {
					targetChangeTicks = 0;
					targetChangeCooldown = mType->info.changeTargetSpeed;

					if (mType->info.changeTargetChance >= uniform_random(1, 100)) {
						if (mType->info.targetDistance <= 1) {
							searchTarget(TARGETSEARCH_RANDOM);
						} else {
							searchTarget(TARGETSEARCH_NEAREST);
						}
					}
				}
			}
		}
	}
}

void Pokemon::onThinkDefense(uint32_t interval)
{
	//
}

void Pokemon::onThinkYell(uint32_t interval)
{
	if (mType->info.yellSpeedTicks == 0) {
		return;
	}

	yellTicks += interval;
	if (yellTicks >= mType->info.yellSpeedTicks) {
		yellTicks = 0;

		if (!mType->info.voiceVector.empty() && (mType->info.yellChance >= static_cast<uint32_t>(uniform_random(1, 100)))) {
			uint32_t index = uniform_random(0, mType->info.voiceVector.size() - 1);
			const voiceBlock_t& vb = mType->info.voiceVector[index];

			if (vb.yellText) {
				g_game.internalCreatureSay(this, TALKTYPE_POKEMON_YELL, vb.text, false);
			} else {
				g_game.internalCreatureSay(this, TALKTYPE_POKEMON_SAY, vb.text, false);
			}

			g_game.addSound(getPosition(), vb.sound);
		}
	}
}

void Pokemon::onThinkEmoticon(uint32_t interval)
{
	if (!belongsToPlayer()) {
		return;
	}

	emotsTicks[0] += interval;
	emotsTicks[1] += interval;
	emotsTicks[2] += interval;

	// low hp pokeball emot
	if (emotsTicks[0] >= 2000) {
		if (getHealth() <= (0.15 * getMaxHealth())) {
			g_game.addEffect(getPosition(), CONST_ME_EMOT_POKEBALL);
			getTrainer()->sendSound(getPosition(), CONST_SE_POKEMONLOWHP, SOUND_CHANNEL_EFFECT);
			emotsTicks[0] = 0;
			return;
		}
	}

	// inside house emot
	if (emotsTicks[1] >= 15000) {
		Tile* tile = g_game.map.getTile(getPosition());
		HouseTile* houseTile = dynamic_cast<HouseTile*>(tile);
		if (houseTile) {
			g_game.addEffect(getPosition(), CONST_ME_EMOT_HOME);
			emotsTicks[1] = 0;
			return;
		}
	}

	// hungry pokemon emot
	if (emotsTicks[2] >= 60000) {
		Condition* condition = getCondition(CONDITION_REGENERATION, CONDITIONID_DEFAULT);
		if (!condition || (condition->getTicks() / 1000) <= 120) {
			g_game.addEffect(getPosition(), CONST_ME_EMOT_FOOD);
			emotsTicks[2] = 0;
			return;
		}
	}
}

void Pokemon::onWalk()
{
	Creature::onWalk();
}

void Pokemon::onWalkComplete()
{
	Creature::onWalkComplete();

	if (checkOrder != nullptr) {
		checkOrder();
		checkOrder = nullptr;
	}
}

bool Pokemon::pushItem(Item* item)
{
	const Position& centerPos = item->getPosition();

	static std::vector<std::pair<int32_t, int32_t>> relList {
		{-1, -1}, {0, -1}, {1, -1},
		{-1,  0},          {1,  0},
		{-1,  1}, {0,  1}, {1,  1}
	};

	std::shuffle(relList.begin(), relList.end(), getRandomGenerator());

	for (const auto& it : relList) {
		Position tryPos(centerPos.x + it.first, centerPos.y + it.second, centerPos.z);
		Tile* tile = g_game.map.getTile(tryPos);
		if (tile && g_game.canThrowObjectTo(centerPos, tryPos)) {
			if (g_game.internalMoveItem(item->getParent(), tile, INDEX_WHEREEVER, item, item->getItemCount(), nullptr) == RETURNVALUE_NOERROR) {
				return true;
			}
		}
	}
	return false;
}

void Pokemon::pushItems(Tile* tile)
{
	//We can not use iterators here since we can push the item to another tile
	//which will invalidate the iterator.
	//start from the end to minimize the amount of traffic
	if (TileItemVector* items = tile->getItemList()) {
		uint32_t moveCount = 0;
		uint32_t removeCount = 0;

		int32_t downItemSize = tile->getDownItemCount();
		for (int32_t i = downItemSize; --i >= 0;) {
			Item* item = items->at(i);
			if (item && item->hasProperty(CONST_PROP_MOVEABLE) && (item->hasProperty(CONST_PROP_BLOCKPATH)
			        || item->hasProperty(CONST_PROP_BLOCKSOLID))) {
				if (moveCount < 20 && Pokemon::pushItem(item)) {
					++moveCount;
				} else if (g_game.internalRemoveItem(item) == RETURNVALUE_NOERROR) {
					++removeCount;
				}
			}
		}

		if (removeCount > 0) {
			g_game.addEffect(tile->getPosition(), CONST_ME_POFF);
		}
	}
}

bool Pokemon::pushCreature(Creature* creature)
{
	static std::vector<Direction> dirList {
			DIRECTION_NORTH,
		DIRECTION_WEST, DIRECTION_EAST,
			DIRECTION_SOUTH
	};
	std::shuffle(dirList.begin(), dirList.end(), getRandomGenerator());

	for (Direction dir : dirList) {
		const Position& tryPos = Moves::getCasterPosition(creature, dir);
		Tile* toTile = g_game.map.getTile(tryPos);
		if (toTile && !toTile->hasFlag(TILESTATE_BLOCKPATH)) {
			if (g_game.internalMoveCreature(creature, dir) == RETURNVALUE_NOERROR) {
				return true;
			}
		}
	}
	return false;
}

void Pokemon::pushCreatures(Tile* tile)
{
	//We can not use iterators here since we can push a creature to another tile
	//which will invalidate the iterator.
	if (CreatureVector* creatures = tile->getCreatures()) {
		uint32_t removeCount = 0;
		Pokemon* lastPushedPokemon = nullptr;

		for (size_t i = 0; i < creatures->size();) {
			Pokemon* pokemon = creatures->at(i)->getPokemon();
			if (pokemon && pokemon->isPushable()) {
				if (pokemon != lastPushedPokemon && Pokemon::pushCreature(pokemon)) {
					lastPushedPokemon = pokemon;
					continue;
				}

				pokemon->changeHealth(-pokemon->getHealth());
				pokemon->setDropLoot(false);
				removeCount++;
			}

			++i;
		}

		if (removeCount > 0) {
			g_game.addEffect(tile->getPosition(), CONST_ME_BLOCKHIT);
		}
	}
}

bool Pokemon::getNextStep(Direction& direction, uint32_t& flags)
{
	if (isIdle || getHealth() <= 0) {
		//we dont have anyone watching might aswell stop walking
		eventWalk = 0;
		return false;
	}

	bool result = false;
	if ((!followCreature || !hasFollowPath) && (!isSummon() || !isMasterInRange)) {
		if (getWalkDelay() <= 0) {
			randomStepping = true;
			//choose a random direction
			result = getRandomStep(getPosition(), direction);
		}
	} else if ((isSummon() && isMasterInRange) || followCreature) {
		randomStepping = false;
		result = Creature::getNextStep(direction, flags);
		if (result) {
			flags |= FLAG_PATHFINDING;
		} else {
			if (ignoreFieldDamage) {
				ignoreFieldDamage = false;
				updateMapCache();
			}
			//target dancing
			if (attackedCreature && attackedCreature == followCreature) {
				if (isFleeing()) {
					result = getDanceStep(getPosition(), direction, false, false);
				} else if (mType->info.staticAttackChance < static_cast<uint32_t>(uniform_random(1, 100))) {
					result = getDanceStep(getPosition(), direction);
				}
			}
		}
	}

	if (result && (canPushItems() || canPushCreatures())) {
		const Position& pos = Moves::getCasterPosition(this, direction);
		Tile* tile = g_game.map.getTile(pos);
		if (tile) {
			if (canPushItems()) {
				Pokemon::pushItems(tile);
			}

			if (canPushCreatures()) {
				Pokemon::pushCreatures(tile);
			}
		}
	}

	return result;
}

bool Pokemon::getRandomStep(const Position& creaturePos, Direction& direction) const
{
	static std::vector<Direction> dirList{
			DIRECTION_NORTH,
		DIRECTION_WEST, DIRECTION_EAST,
			DIRECTION_SOUTH
	};
	std::shuffle(dirList.begin(), dirList.end(), getRandomGenerator());

	for (Direction dir : dirList) {
		if (canWalkTo(creaturePos, dir)) {
			direction = dir;
			return true;
		}
	}
	return false;
}

bool Pokemon::getDanceStep(const Position& creaturePos, Direction& direction,
                           bool keepAttack /*= true*/, bool keepDistance /*= true*/)
{
	bool canDoAttackNow = canUseAttack(creaturePos, attackedCreature);

	assert(attackedCreature != nullptr);
	const Position& centerPos = attackedCreature->getPosition();

	int_fast32_t offset_x = Position::getOffsetX(creaturePos, centerPos);
	int_fast32_t offset_y = Position::getOffsetY(creaturePos, centerPos);

	int_fast32_t distance_x = std::abs(offset_x);
	int_fast32_t distance_y = std::abs(offset_y);

	uint32_t centerToDist = std::max<uint32_t>(distance_x, distance_y);

	std::vector<Direction> dirList;

	if (!keepDistance || offset_y >= 0) {
		uint32_t tmpDist = std::max<uint32_t>(distance_x, std::abs((creaturePos.getY() - 1) - centerPos.getY()));
		if (tmpDist == centerToDist && canWalkTo(creaturePos, DIRECTION_NORTH)) {
			bool result = true;

			if (keepAttack) {
				result = (!canDoAttackNow || canUseAttack(Position(creaturePos.x, creaturePos.y - 1, creaturePos.z), attackedCreature));
			}

			if (result) {
				dirList.push_back(DIRECTION_NORTH);
			}
		}
	}

	if (!keepDistance || offset_y <= 0) {
		uint32_t tmpDist = std::max<uint32_t>(distance_x, std::abs((creaturePos.getY() + 1) - centerPos.getY()));
		if (tmpDist == centerToDist && canWalkTo(creaturePos, DIRECTION_SOUTH)) {
			bool result = true;

			if (keepAttack) {
				result = (!canDoAttackNow || canUseAttack(Position(creaturePos.x, creaturePos.y + 1, creaturePos.z), attackedCreature));
			}

			if (result) {
				dirList.push_back(DIRECTION_SOUTH);
			}
		}
	}

	if (!keepDistance || offset_x <= 0) {
		uint32_t tmpDist = std::max<uint32_t>(std::abs((creaturePos.getX() + 1) - centerPos.getX()), distance_y);
		if (tmpDist == centerToDist && canWalkTo(creaturePos, DIRECTION_EAST)) {
			bool result = true;

			if (keepAttack) {
				result = (!canDoAttackNow || canUseAttack(Position(creaturePos.x + 1, creaturePos.y, creaturePos.z), attackedCreature));
			}

			if (result) {
				dirList.push_back(DIRECTION_EAST);
			}
		}
	}

	if (!keepDistance || offset_x >= 0) {
		uint32_t tmpDist = std::max<uint32_t>(std::abs((creaturePos.getX() - 1) - centerPos.getX()), distance_y);
		if (tmpDist == centerToDist && canWalkTo(creaturePos, DIRECTION_WEST)) {
			bool result = true;

			if (keepAttack) {
				result = (!canDoAttackNow || canUseAttack(Position(creaturePos.x - 1, creaturePos.y, creaturePos.z), attackedCreature));
			}

			if (result) {
				dirList.push_back(DIRECTION_WEST);
			}
		}
	}

	if (!dirList.empty()) {
		std::shuffle(dirList.begin(), dirList.end(), getRandomGenerator());
		direction = dirList[uniform_random(0, dirList.size() - 1)];
		return true;
	}
	return false;
}

bool Pokemon::getDistanceStep(const Position& targetPos, Direction& direction, bool flee /* = false */)
{
	const Position& creaturePos = getPosition();

	int_fast32_t dx = Position::getDistanceX(creaturePos, targetPos);
	int_fast32_t dy = Position::getDistanceY(creaturePos, targetPos);

	int32_t distance = std::max<int32_t>(dx, dy);

	if (!flee && (distance > mType->info.targetDistance || !g_game.isSightClear(creaturePos, targetPos, true))) {
		return false; // let the A* calculate it
	} else if (!flee && distance == mType->info.targetDistance) {
		return true; // we don't really care here, since it's what we wanted to reach (a dancestep will take of dancing in that position)
	}

	int_fast32_t offsetx = Position::getOffsetX(creaturePos, targetPos);
	int_fast32_t offsety = Position::getOffsetY(creaturePos, targetPos);

	if (dx <= 1 && dy <= 1) {
		//seems like a target is near, it this case we need to slow down our movements (as a pokemon)
		if (stepDuration < 2) {
			stepDuration++;
		}
	} else if (stepDuration > 0) {
		stepDuration--;
	}

	if (offsetx == 0 && offsety == 0) {
		return getRandomStep(creaturePos, direction); // player is "on" the pokemon so let's get some random step and rest will be taken care later.
	}

	if (dx == dy) {
		//player is diagonal to the pokemon
		if (offsetx >= 1 && offsety >= 1) {
			// player is NW
			//escape to SE, S or E [and some extra]
			bool s = canWalkTo(creaturePos, DIRECTION_SOUTH);
			bool e = canWalkTo(creaturePos, DIRECTION_EAST);

			if (s && e) {
				direction = boolean_random() ? DIRECTION_SOUTH : DIRECTION_EAST;
				return true;
			} else if (s) {
				direction = DIRECTION_SOUTH;
				return true;
			} else if (e) {
				direction = DIRECTION_EAST;
				return true;
			} else if (canWalkTo(creaturePos, DIRECTION_SOUTHEAST)) {
				direction = DIRECTION_SOUTHEAST;
				return true;
			}

			/* fleeing */
			bool n = canWalkTo(creaturePos, DIRECTION_NORTH);
			bool w = canWalkTo(creaturePos, DIRECTION_WEST);

			if (flee) {
				if (n && w) {
					direction = boolean_random() ? DIRECTION_NORTH : DIRECTION_WEST;
					return true;
				} else if (n) {
					direction = DIRECTION_NORTH;
					return true;
				} else if (w) {
					direction = DIRECTION_WEST;
					return true;
				}
			}

			/* end of fleeing */

			if (w && canWalkTo(creaturePos, DIRECTION_SOUTHWEST)) {
				direction = DIRECTION_WEST;
			} else if (n && canWalkTo(creaturePos, DIRECTION_NORTHEAST)) {
				direction = DIRECTION_NORTH;
			}

			return true;
		} else if (offsetx <= -1 && offsety <= -1) {
			//player is SE
			//escape to NW , W or N [and some extra]
			bool w = canWalkTo(creaturePos, DIRECTION_WEST);
			bool n = canWalkTo(creaturePos, DIRECTION_NORTH);

			if (w && n) {
				direction = boolean_random() ? DIRECTION_WEST : DIRECTION_NORTH;
				return true;
			} else if (w) {
				direction = DIRECTION_WEST;
				return true;
			} else if (n) {
				direction = DIRECTION_NORTH;
				return true;
			}

			if (canWalkTo(creaturePos, DIRECTION_NORTHWEST)) {
				direction = DIRECTION_NORTHWEST;
				return true;
			}

			/* fleeing */
			bool s = canWalkTo(creaturePos, DIRECTION_SOUTH);
			bool e = canWalkTo(creaturePos, DIRECTION_EAST);

			if (flee) {
				if (s && e) {
					direction = boolean_random() ? DIRECTION_SOUTH : DIRECTION_EAST;
					return true;
				} else if (s) {
					direction = DIRECTION_SOUTH;
					return true;
				} else if (e) {
					direction = DIRECTION_EAST;
					return true;
				}
			}

			/* end of fleeing */

			if (s && canWalkTo(creaturePos, DIRECTION_SOUTHWEST)) {
				direction = DIRECTION_SOUTH;
			} else if (e && canWalkTo(creaturePos, DIRECTION_NORTHEAST)) {
				direction = DIRECTION_EAST;
			}

			return true;
		} else if (offsetx >= 1 && offsety <= -1) {
			//player is SW
			//escape to NE, N, E [and some extra]
			bool n = canWalkTo(creaturePos, DIRECTION_NORTH);
			bool e = canWalkTo(creaturePos, DIRECTION_EAST);
			if (n && e) {
				direction = boolean_random() ? DIRECTION_NORTH : DIRECTION_EAST;
				return true;
			} else if (n) {
				direction = DIRECTION_NORTH;
				return true;
			} else if (e) {
				direction = DIRECTION_EAST;
				return true;
			}

			if (canWalkTo(creaturePos, DIRECTION_NORTHEAST)) {
				direction = DIRECTION_NORTHEAST;
				return true;
			}

			/* fleeing */
			bool s = canWalkTo(creaturePos, DIRECTION_SOUTH);
			bool w = canWalkTo(creaturePos, DIRECTION_WEST);

			if (flee) {
				if (s && w) {
					direction = boolean_random() ? DIRECTION_SOUTH : DIRECTION_WEST;
					return true;
				} else if (s) {
					direction = DIRECTION_SOUTH;
					return true;
				} else if (w) {
					direction = DIRECTION_WEST;
					return true;
				}
			}

			/* end of fleeing */

			if (w && canWalkTo(creaturePos, DIRECTION_NORTHWEST)) {
				direction = DIRECTION_WEST;
			} else if (s && canWalkTo(creaturePos, DIRECTION_SOUTHEAST)) {
				direction = DIRECTION_SOUTH;
			}

			return true;
		} else if (offsetx <= -1 && offsety >= 1) {
			// player is NE
			//escape to SW, S, W [and some extra]
			bool w = canWalkTo(creaturePos, DIRECTION_WEST);
			bool s = canWalkTo(creaturePos, DIRECTION_SOUTH);
			if (w && s) {
				direction = boolean_random() ? DIRECTION_WEST : DIRECTION_SOUTH;
				return true;
			} else if (w) {
				direction = DIRECTION_WEST;
				return true;
			} else if (s) {
				direction = DIRECTION_SOUTH;
				return true;
			} else if (canWalkTo(creaturePos, DIRECTION_SOUTHWEST)) {
				direction = DIRECTION_SOUTHWEST;
				return true;
			}

			/* fleeing */
			bool n = canWalkTo(creaturePos, DIRECTION_NORTH);
			bool e = canWalkTo(creaturePos, DIRECTION_EAST);

			if (flee) {
				if (n && e) {
					direction = boolean_random() ? DIRECTION_NORTH : DIRECTION_EAST;
					return true;
				} else if (n) {
					direction = DIRECTION_NORTH;
					return true;
				} else if (e) {
					direction = DIRECTION_EAST;
					return true;
				}
			}

			/* end of fleeing */

			if (e && canWalkTo(creaturePos, DIRECTION_SOUTHEAST)) {
				direction = DIRECTION_EAST;
			} else if (n && canWalkTo(creaturePos, DIRECTION_NORTHWEST)) {
				direction = DIRECTION_NORTH;
			}

			return true;
		}
	}

	//Now let's decide where the player is located to the pokemon (what direction) so we can decide where to escape.
	if (dy > dx) {
		Direction playerDir = offsety < 0 ? DIRECTION_SOUTH : DIRECTION_NORTH;
		switch (playerDir) {
			case DIRECTION_NORTH: {
				// Player is to the NORTH, so obviously we need to check if we can go SOUTH, if not then let's choose WEST or EAST and again if we can't we need to decide about some diagonal movements.
				if (canWalkTo(creaturePos, DIRECTION_SOUTH)) {
					direction = DIRECTION_SOUTH;
					return true;
				}

				bool w = canWalkTo(creaturePos, DIRECTION_WEST);
				bool e = canWalkTo(creaturePos, DIRECTION_EAST);
				if (w && e && offsetx == 0) {
					direction = boolean_random() ? DIRECTION_WEST : DIRECTION_EAST;
					return true;
				} else if (w && offsetx <= 0) {
					direction = DIRECTION_WEST;
					return true;
				} else if (e && offsetx >= 0) {
					direction = DIRECTION_EAST;
					return true;
				}

				/* fleeing */
				if (flee) {
					if (w && e) {
						direction = boolean_random() ? DIRECTION_WEST : DIRECTION_EAST;
						return true;
					} else if (w) {
						direction = DIRECTION_WEST;
						return true;
					} else if (e) {
						direction = DIRECTION_EAST;
						return true;
					}
				}

				/* end of fleeing */

				bool sw = canWalkTo(creaturePos, DIRECTION_SOUTHWEST);
				bool se = canWalkTo(creaturePos, DIRECTION_SOUTHEAST);
				if (sw || se) {
					// we can move both dirs
					if (sw && se) {
						direction = boolean_random() ? DIRECTION_SOUTHWEST : DIRECTION_SOUTHEAST;
					} else if (w) {
						direction = DIRECTION_WEST;
					} else if (sw) {
						direction = DIRECTION_SOUTHWEST;
					} else if (e) {
						direction = DIRECTION_EAST;
					} else if (se) {
						direction = DIRECTION_SOUTHEAST;
					}
					return true;
				}

				/* fleeing */
				if (flee && canWalkTo(creaturePos, DIRECTION_NORTH)) {
					// towards player, yea
					direction = DIRECTION_NORTH;
					return true;
				}

				/* end of fleeing */
				break;
			}

			case DIRECTION_SOUTH: {
				if (canWalkTo(creaturePos, DIRECTION_NORTH)) {
					direction = DIRECTION_NORTH;
					return true;
				}

				bool w = canWalkTo(creaturePos, DIRECTION_WEST);
				bool e = canWalkTo(creaturePos, DIRECTION_EAST);
				if (w && e && offsetx == 0) {
					direction = boolean_random() ? DIRECTION_WEST : DIRECTION_EAST;
					return true;
				} else if (w && offsetx <= 0) {
					direction = DIRECTION_WEST;
					return true;
				} else if (e && offsetx >= 0) {
					direction = DIRECTION_EAST;
					return true;
				}

				/* fleeing */
				if (flee) {
					if (w && e) {
						direction = boolean_random() ? DIRECTION_WEST : DIRECTION_EAST;
						return true;
					} else if (w) {
						direction = DIRECTION_WEST;
						return true;
					} else if (e) {
						direction = DIRECTION_EAST;
						return true;
					}
				}

				/* end of fleeing */

				bool nw = canWalkTo(creaturePos, DIRECTION_NORTHWEST);
				bool ne = canWalkTo(creaturePos, DIRECTION_NORTHEAST);
				if (nw || ne) {
					// we can move both dirs
					if (nw && ne) {
						direction = boolean_random() ? DIRECTION_NORTHWEST : DIRECTION_NORTHEAST;
					} else if (w) {
						direction = DIRECTION_WEST;
					} else if (nw) {
						direction = DIRECTION_NORTHWEST;
					} else if (e) {
						direction = DIRECTION_EAST;
					} else if (ne) {
						direction = DIRECTION_NORTHEAST;
					}
					return true;
				}

				/* fleeing */
				if (flee && canWalkTo(creaturePos, DIRECTION_SOUTH)) {
					// towards player, yea
					direction = DIRECTION_SOUTH;
					return true;
				}

				/* end of fleeing */
				break;
			}

			default:
				break;
		}
	} else {
		Direction playerDir = offsetx < 0 ? DIRECTION_EAST : DIRECTION_WEST;
		switch (playerDir) {
			case DIRECTION_WEST: {
				if (canWalkTo(creaturePos, DIRECTION_EAST)) {
					direction = DIRECTION_EAST;
					return true;
				}

				bool n = canWalkTo(creaturePos, DIRECTION_NORTH);
				bool s = canWalkTo(creaturePos, DIRECTION_SOUTH);
				if (n && s && offsety == 0) {
					direction = boolean_random() ? DIRECTION_NORTH : DIRECTION_SOUTH;
					return true;
				} else if (n && offsety <= 0) {
					direction = DIRECTION_NORTH;
					return true;
				} else if (s && offsety >= 0) {
					direction = DIRECTION_SOUTH;
					return true;
				}

				/* fleeing */
				if (flee) {
					if (n && s) {
						direction = boolean_random() ? DIRECTION_NORTH : DIRECTION_SOUTH;
						return true;
					} else if (n) {
						direction = DIRECTION_NORTH;
						return true;
					} else if (s) {
						direction = DIRECTION_SOUTH;
						return true;
					}
				}

				/* end of fleeing */

				bool se = canWalkTo(creaturePos, DIRECTION_SOUTHEAST);
				bool ne = canWalkTo(creaturePos, DIRECTION_NORTHEAST);
				if (se || ne) {
					if (se && ne) {
						direction = boolean_random() ? DIRECTION_SOUTHEAST : DIRECTION_NORTHEAST;
					} else if (s) {
						direction = DIRECTION_SOUTH;
					} else if (se) {
						direction = DIRECTION_SOUTHEAST;
					} else if (n) {
						direction = DIRECTION_NORTH;
					} else if (ne) {
						direction = DIRECTION_NORTHEAST;
					}
					return true;
				}

				/* fleeing */
				if (flee && canWalkTo(creaturePos, DIRECTION_WEST)) {
					// towards player, yea
					direction = DIRECTION_WEST;
					return true;
				}

				/* end of fleeing */
				break;
			}

			case DIRECTION_EAST: {
				if (canWalkTo(creaturePos, DIRECTION_WEST)) {
					direction = DIRECTION_WEST;
					return true;
				}

				bool n = canWalkTo(creaturePos, DIRECTION_NORTH);
				bool s = canWalkTo(creaturePos, DIRECTION_SOUTH);
				if (n && s && offsety == 0) {
					direction = boolean_random() ? DIRECTION_NORTH : DIRECTION_SOUTH;
					return true;
				} else if (n && offsety <= 0) {
					direction = DIRECTION_NORTH;
					return true;
				} else if (s && offsety >= 0) {
					direction = DIRECTION_SOUTH;
					return true;
				}

				/* fleeing */
				if (flee) {
					if (n && s) {
						direction = boolean_random() ? DIRECTION_NORTH : DIRECTION_SOUTH;
						return true;
					} else if (n) {
						direction = DIRECTION_NORTH;
						return true;
					} else if (s) {
						direction = DIRECTION_SOUTH;
						return true;
					}
				}

				/* end of fleeing */

				bool nw = canWalkTo(creaturePos, DIRECTION_NORTHWEST);
				bool sw = canWalkTo(creaturePos, DIRECTION_SOUTHWEST);
				if (nw || sw) {
					if (nw && sw) {
						direction = boolean_random() ? DIRECTION_NORTHWEST : DIRECTION_SOUTHWEST;
					} else if (n) {
						direction = DIRECTION_NORTH;
					} else if (nw) {
						direction = DIRECTION_NORTHWEST;
					} else if (s) {
						direction = DIRECTION_SOUTH;
					} else if (sw) {
						direction = DIRECTION_SOUTHWEST;
					}
					return true;
				}

				/* fleeing */
				if (flee && canWalkTo(creaturePos, DIRECTION_EAST)) {
					// towards player, yea
					direction = DIRECTION_EAST;
					return true;
				}

				/* end of fleeing */
				break;
			}

			default:
				break;
		}
	}

	return true;
}

bool Pokemon::canWalkTo(Position pos, Direction direction) const
{
	pos = getNextPosition(direction, pos);
	if (isInSpawnRange(pos)) {
		if (getWalkCache(pos) == 0) {
			return false;
		}

		Tile* tile = g_game.map.getTile(pos);
		if (tile && tile->getTopVisibleCreature(this) == nullptr && tile->queryAdd(0, *this, 1, FLAG_PATHFINDING) == RETURNVALUE_NOERROR) {
			return true;
		}
	}
	return false;
}

bool Pokemon::teleportToPlayer()
{
	Position pokemonPosition = getPosition();
	Position trainerPosition = getMaster()->getPosition();
	if (g_game.internalTeleport(this, trainerPosition) != RETURNVALUE_NOERROR) {
		return false;
	}

	g_game.addEffect(pokemonPosition, CONST_ME_POFF);
	g_game.addEffect(getPosition(), CONST_ME_TELEPORT);
	isMasterInRange = true;
	needTeleportToPlayer = false;
	return true;
}

void Pokemon::death(Creature*)
{
	setAttackedCreature(nullptr);

	for (Creature* summon : summons) {
		summon->changeHealth(-summon->getHealth());
		summon->removeMaster();
	}
	summons.clear();

	clearTargetList();
	clearFriendList();
	onIdleStatus();
}

Item* Pokemon::getCorpse(Creature* lastHitCreature, Creature* mostDamageCreature)
{
	Item* corpse;

	if (getDittoStatus()) {
		if (getShinyStatus()) {
			corpse = Item::CreateItem(ITEM_DITTO_CORPSE); // Shiny Ditto Corpse
		} else {
			corpse = Item::CreateItem(ITEM_DITTO_CORPSE); // Ditto Corpse
		}
	} else{
		if (getShinyStatus()) {
			corpse = Item::CreateItem(mType->info.shiny.corpse); // Shiny Corpse
		} else {
			corpse = Creature::getCorpse(lastHitCreature, mostDamageCreature); // Normal Corpse
		}
	}

	if (corpse) {
		if (mostDamageCreature) {
			if (mostDamageCreature->getPlayer()) {
				corpse->setCorpseOwner(mostDamageCreature->getID());
			} else {
				const Creature* mostDamageCreatureMaster = mostDamageCreature->getMaster();
				if (mostDamageCreatureMaster && mostDamageCreatureMaster->getPlayer()) {
					corpse->setCorpseOwner(mostDamageCreatureMaster->getID());
				}
			}
		}
		corpse->setPokemonCorpseGender(getGender());
		corpse->setPokemonCorpseType(mType->typeName);
		corpse->setPokemonCorpseShinyStatus(getShinyStatus());
		corpse->setPokemonCorpseNature(getNature());
	}

	return corpse;
}

bool Pokemon::isInSpawnRange(const Position& pos) const
{
	if (!spawn) {
		return true;
	}

	if (Pokemon::despawnRadius == 0) {
		return true;
	}

	if (!Spawns::isInZone(masterPos, Pokemon::despawnRadius, pos)) {
		return false;
	}

	if (Pokemon::despawnRange == 0) {
		return true;
	}

	if (Position::getDistanceZ(pos, masterPos) > Pokemon::despawnRange) {
		return false;
	}

	return true;
}

bool Pokemon::getCombatValues(int32_t& min, int32_t& max)
{
	if (minCombatValue == 0 && maxCombatValue == 0) {
		return false;
	}

	min = minCombatValue;
	max = maxCombatValue;
	return true;
}

void Pokemon::updateLookDirection()
{
	Direction newDir = getDirection();

	if (attackedCreature) {
		const Position& pos = getPosition();
		const Position& attackedCreaturePos = attackedCreature->getPosition();
		int_fast32_t offsetx = Position::getOffsetX(attackedCreaturePos, pos);
		int_fast32_t offsety = Position::getOffsetY(attackedCreaturePos, pos);

		int32_t dx = std::abs(offsetx);
		int32_t dy = std::abs(offsety);
		if (dx > dy) {
			//look EAST/WEST
			if (offsetx < 0) {
				newDir = DIRECTION_WEST;
			} else {
				newDir = DIRECTION_EAST;
			}
		} else if (dx < dy) {
			//look NORTH/SOUTH
			if (offsety < 0) {
				newDir = DIRECTION_NORTH;
			} else {
				newDir = DIRECTION_SOUTH;
			}
		} else {
			Direction dir = getDirection();
			if (offsetx < 0 && offsety < 0) {
				if (dir == DIRECTION_SOUTH) {
					newDir = DIRECTION_WEST;
				} else if (dir == DIRECTION_EAST) {
					newDir = DIRECTION_NORTH;
				}
			} else if (offsetx < 0 && offsety > 0) {
				if (dir == DIRECTION_NORTH) {
					newDir = DIRECTION_WEST;
				} else if (dir == DIRECTION_EAST) {
					newDir = DIRECTION_SOUTH;
				}
			} else if (offsetx > 0 && offsety < 0) {
				if (dir == DIRECTION_SOUTH) {
					newDir = DIRECTION_EAST;
				} else if (dir == DIRECTION_WEST) {
					newDir = DIRECTION_NORTH;
				}
			} else {
				if (dir == DIRECTION_NORTH) {
					newDir = DIRECTION_EAST;
				} else if (dir == DIRECTION_WEST) {
					newDir = DIRECTION_SOUTH;
				}
			}
		}
	}

	g_game.internalCreatureTurn(this, newDir);
}

void Pokemon::dropLoot(Container* corpse, Creature*)
{
	if (corpse && lootDrop) {
		mType->createLoot(corpse);
	}
}

void Pokemon::setNormalCreatureLight()
{
	internalLight = mType->info.light;
}

void Pokemon::drainHealth(Creature* attacker, int32_t damage)
{
	Creature::drainHealth(attacker, damage);

	if (damage > 0 && randomStepping) {
		ignoreFieldDamage = true;
		updateMapCache();
	}

	if (isInvisible()) {
		removeCondition(CONDITION_INVISIBLE);
	}
}

void Pokemon::changeHealth(int32_t healthChange, bool sendHealthChange/* = true*/)
{
	//In case a player with ignore flag set attacks the pokemon
	setIdle(false);
	Creature::changeHealth(healthChange, sendHealthChange);
}

bool Pokemon::challengeCreature(Creature* creature)
{
	if (isSummon()) {
		return false;
	}

	bool result = selectTarget(creature);
	if (result) {
		targetChangeCooldown = 8000;
		targetChangeTicks = 0;
	}
	return result;
}

void Pokemon::getPathSearchParams(const Creature* creature, FindPathParams& fpp) const
{
	Creature::getPathSearchParams(creature, fpp);

	fpp.minTargetDist = 1;
	fpp.maxTargetDist = mType->info.targetDistance;

	if (isSummon()) {
		if (getMaster() == creature) {
			fpp.maxTargetDist = 2;
			fpp.fullPathSearch = true;
		} else if (mType->info.targetDistance <= 1) {
			fpp.fullPathSearch = true;
		} else {
			fpp.fullPathSearch = !canUseAttack(getPosition(), creature);
		}
	} else if (isFleeing()) {
		//Distance should be higher than the client view range (Map::maxClientViewportX/Map::maxClientViewportY)
		fpp.maxTargetDist = Map::maxViewportX;
		fpp.clearSight = false;
		fpp.keepDistance = true;
		fpp.fullPathSearch = false;
	} else if (mType->info.targetDistance <= 1) {
		fpp.fullPathSearch = true;
	} else {
		fpp.fullPathSearch = !canUseAttack(getPosition(), creature);
	}
}

bool Pokemon::checkSpawn()
{
	if (!belongsToPlayer() && getPokemonType()->info.spawnAt != 0) {
		if (getPokemonType()->info.spawnAt == 1 && !getTile()->hasFlag(TILESTATE_CAVE)) {
			if (!g_game.isNight() && !g_game.isSunset()) {
				g_game.addEffect(getPosition(), 3);
				g_game.internalCreatureSay(this, TALKTYPE_POKEMON_SAY, getName() + " ran away...", false);
				g_game.removeCreature(this, true);
				return true;
			}
		} else if (getPokemonType()->info.spawnAt == 2 && !g_game.isDay() && !g_game.isSunrise()) {
			g_game.addEffect(getPosition(), 3);
			g_game.internalCreatureSay(this, TALKTYPE_POKEMON_SAY, getName() + " ran away...", false);
			g_game.removeCreature(this, true);
			return true;
		}
	}
	return false;
}

void Pokemon::setPokemonType(PokemonType* pmType)
{
	if (mType == pmType) {
		return;
	}

	if (mType && name == mType->name) {
		g_game.internalCreatureChangeName(this, pmType->name);
	}

	mType = pmType;
	strDescription = pmType->nameDescription;

	if (isShiny && (pmType->info.shiny.outfit.lookType)) {
		g_game.internalCreatureChangeOutfit(this, pmType->info.shiny.outfit);
		defaultOutfit = pmType->info.shiny.outfit;
	} else {
		g_game.internalCreatureChangeOutfit(this, pmType->info.outfit);
		defaultOutfit = pmType->info.outfit;
	}
}

float Pokemon::getAttackMultiplier() const
{
	if (nature == NATURE_LONELY || nature == NATURE_ADAMANT || nature == NATURE_NAUGHTY ||
	     nature == NATURE_BRAVE) {
		return 1.1;
	} else if (nature == NATURE_BOLD || nature == NATURE_MODEST || nature == NATURE_CALM ||
		         nature == NATURE_TIMID) {
		return 0.9;
	} else {
		return 1;
	}
}


float Pokemon::getDefenseMultiplier() const
{
	if (nature == NATURE_BOLD || nature == NATURE_IMPISH || nature == NATURE_LAX ||
	     nature == NATURE_RELAXED) {
		return 1.1;
	} else if (nature == NATURE_LONELY || nature == NATURE_MILD || nature == NATURE_GENTLE ||
	            nature == NATURE_HASTY) {
		return 0.9;
	} else {
		return 1;
	}
}

float Pokemon::getSpecialAttackMultiplier() const
{
	if (nature == NATURE_MODEST || nature == NATURE_MILD || nature == NATURE_RASH ||
	     nature == NATURE_QUIET) {
		return 1.1;
	} else if (nature == NATURE_ADAMANT || nature == NATURE_IMPISH || nature == NATURE_CAREFUL ||
			    nature == NATURE_JOLLY) {
		return 0.9;
	} else {
		return 1;
	}
}

float Pokemon::getSpecialDefenseMultiplier() const
{
	if (nature == NATURE_CALM || nature == NATURE_GENTLE || nature == NATURE_CAREFUL ||
	     nature == NATURE_SASSY) {
		return 1.1;
	} else if (nature == NATURE_NAUGHTY || nature == NATURE_LAX || nature == NATURE_RASH ||
	            nature == NATURE_NAIVE) {
		return 0.9;
	} else {
		return 1;
	}
}

float Pokemon::getSpeedMultiplier() const
{
	if (nature == NATURE_TIMID || nature == NATURE_HASTY || nature == NATURE_JOLLY ||
	     nature == NATURE_NAIVE) {
		return 1.1;
	} else if (nature == NATURE_BRAVE || nature == NATURE_RELAXED || nature == NATURE_QUIET ||
	            nature == NATURE_SASSY) {
		return 0.9;
	} else {
		return 1;
	}
}

bool Pokemon::moveTo(const Position& pos, int32_t minTargetDist /* = 0 */, int32_t maxTargetDist /* = 0 */)
{
	FindPathParams fpp;
	fpp.fullPathSearch = true;
	fpp.clearSight = true;
	fpp.maxSearchDist = 30;
	fpp.minTargetDist = minTargetDist;
	fpp.maxTargetDist = maxTargetDist;

	std::forward_list<Direction> listDir;
	if (getPathTo(pos, listDir, fpp)) {
		hasFollowPath = true;
		followMaster = false;
		setFollowCreature(nullptr);
		startAutoWalk(listDir);
		return true;
	}

	return false;
}

void Pokemon::checkCutDigOrRockSmash(Item* item) {
	if (!item && !item->isCuttable() && !item->isSmashable() && !item->isDiggable()) {
		return;
	}

	const Position& pokemonPos = getPosition();
	const Position& itemPos = item->getPosition();

	if ((sqrt(pow((pokemonPos.x - itemPos.x), 2) + pow((pokemonPos.y - itemPos.y), 2))) < 1.5f) {
		if (item->isCuttable()) {
			g_game.addEffect(item->getPosition(), 292);
		} else if (item->isSmashable()) {
			g_game.addEffect(item->getPosition(), 292);
		}

		turnToThing(item);
		g_game.transformItem(item, Item::items[item->getID()].destroyTo);
		item->startDecaying();
	}
}

void Pokemon::turnToThing(Thing* thing)
{
	const Position& thingPos = thing->getPosition();
	const Position& myPos = getPosition();
	const auto dx = Position::getOffsetX(myPos, thingPos);
	const auto dy = Position::getOffsetY(myPos, thingPos);

	float tan;
	if (dx != 0) {
		tan = static_cast<float>(dy) / dx;
	} else {
		tan = 10;
	}

	Direction dir;
	if (std::abs(tan) < 1) {
		if (dx > 0) {
			dir = DIRECTION_WEST;
		} else {
			dir = DIRECTION_EAST;
		}
	} else {
		if (dy > 0) {
			dir = DIRECTION_NORTH;
		} else {
			dir = DIRECTION_SOUTH;
		}
	}
	g_game.internalCreatureTurn(this, dir);
}

void Pokemon::transform(PokemonType* pmType)
{
	setPokemonType(pmType);
	setHealth(getMaxHealth());
	cleanConditions();
}

bool Pokemon::setMaster(Creature* newMaster)
{
	if (!Creature::setMaster(newMaster)) {
		return false;
	}

	if (!newMaster) {
		return false;
	}

	if (!newMaster->getPlayer() && !newMaster->getPokemon()) {
		return true;
	}

	Player* player = newMaster->getPlayer();
	setLevel(player->getLevel());

	if (getHealth() > getMaxHealth()) {
		setHealth(getMaxHealth());
	}

	return true;
}