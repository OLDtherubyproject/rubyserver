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

#include "game.h"
#include "configmanager.h"
#include "events.h"

extern Game g_game;
extern ConfigManager g_config;
extern Events* g_events;

CombatDamage Combat::getCombatDamage(Creature* creature, Creature* target) const
{
	CombatDamage damage;
	damage.origin = params.origin;
	damage.type = params.combatType;

	if (creature) {
		if (Pokemon* pokemon = creature->getPokemon()) {
			if (params.valueCallback) {
				params.valueCallback->getDamageValue(pokemon, damage);
			}
		}
	}

	return damage;
}

void Combat::getCombatArea(const Position& centerPos, const Position& targetPos, const AreaCombat* area, std::forward_list<Tile*>& list)
{
	if (targetPos.z >= MAP_MAX_LAYERS) {
		return;
	}

	if (area) {
		area->getList(centerPos, targetPos, list);
	} else {
		Tile* tile = g_game.map.getTile(targetPos);
		if (!tile) {
			tile = new StaticTile(targetPos.x, targetPos.y, targetPos.z);
			g_game.map.setTile(targetPos, tile);
		}
		list.push_front(tile);
	}
}

CombatType_t Combat::ConditionToDamageType(ConditionType_t type)
{
	switch (type) {
		case CONDITION_FIRE:
			return COMBAT_FIREDAMAGE;

		case CONDITION_ENERGY:
			return COMBAT_ELECTRICDAMAGE;

		case CONDITION_BLEEDING:
			return COMBAT_PHYSICALDAMAGE;

		case CONDITION_DROWN:
			return COMBAT_WATERDAMAGE;

		case CONDITION_POISON:
			return COMBAT_GRASSDAMAGE;

		case CONDITION_FREEZING:
			return COMBAT_ICEDAMAGE;

		case CONDITION_DAZZLED:
			return COMBAT_BUGDAMAGE;

		case CONDITION_CURSED:
			return COMBAT_DARKDAMAGE;

		default:
			break;
	}

	return COMBAT_NONE;
}

ConditionType_t Combat::DamageToConditionType(CombatType_t type)
{
	switch (type) {
		case COMBAT_FIREDAMAGE:
			return CONDITION_FIRE;

		case COMBAT_ELECTRICDAMAGE:
			return CONDITION_ENERGY;

		case COMBAT_WATERDAMAGE:
			return CONDITION_DROWN;

		case COMBAT_GRASSDAMAGE:
			return CONDITION_POISON;

		case COMBAT_ICEDAMAGE:
			return CONDITION_FREEZING;

		case COMBAT_BUGDAMAGE:
			return CONDITION_DAZZLED;

		case COMBAT_DARKDAMAGE:
			return CONDITION_CURSED;

		case COMBAT_PHYSICALDAMAGE:
			return CONDITION_BLEEDING;

		default:
			return CONDITION_NONE;
	}
}

bool Combat::isPlayerCombat(const Creature* target)
{
	if (target->getPlayer()) {
		return true;
	}

	if (target->isSummon() && target->getMaster()->getPlayer()) {
		return true;
	}

	return false;
}

ReturnValue Combat::canTargetCreature(Player* attacker, Creature* target)
{
	if (attacker == target) {
		return RETURNVALUE_YOUMAYNOTATTACKTHISPLAYER;
	}

	if (!attacker->hasFlag(PlayerFlag_IgnoreProtectionZone)) {
		//pz-zone
		if (attacker->getZone() == ZONE_PROTECTION) {
			return RETURNVALUE_YOUMAYNOTATTACKAPERSONWHILEINPROTECTIONZONE;
		}

		if (target->getZone() == ZONE_PROTECTION) {
			return RETURNVALUE_YOUMAYNOTATTACKAPERSONINPROTECTIONZONE;
		}

		//nopvp-zone
		if (isPlayerCombat(target)) {
			if (attacker->getZone() == ZONE_NOPVP) {
				return RETURNVALUE_ACTIONNOTPERMITTEDINANOPVPZONE;
			}

			if (target->getZone() == ZONE_NOPVP) {
				return RETURNVALUE_YOUMAYNOTATTACKAPERSONINPROTECTIONZONE;
			}
		}
	}

	if (attacker->hasFlag(PlayerFlag_CannotUseCombat) || !target->isAttackable()) {
		if (target->getPlayer()) {
			return RETURNVALUE_YOUMAYNOTATTACKTHISPLAYER;
		} else {
			return RETURNVALUE_YOUMAYNOTATTACKTHISCREATURE;
		}
	}

	if (target->getPlayer()) {
		if (isProtected(attacker, target->getPlayer())) {
			return RETURNVALUE_YOUMAYNOTATTACKTHISPLAYER;
		}
	}

	return Combat::canDoCombat(attacker, target);
}

ReturnValue Combat::canDoCombat(Creature* caster, Tile* tile, bool aggressive)
{
	if (tile->hasProperty(CONST_PROP_BLOCKPROJECTILE)) {
		return RETURNVALUE_NOTENOUGHROOM;
	}

	if (tile->hasFlag(TILESTATE_FLOORCHANGE)) {
		return RETURNVALUE_NOTENOUGHROOM;
	}

	if (tile->getTeleportItem()) {
		return RETURNVALUE_NOTENOUGHROOM;
	}

	if (caster) {
		const Position& casterPosition = caster->getPosition();
		const Position& tilePosition = tile->getPosition();
		if (casterPosition.z < tilePosition.z) {
			return RETURNVALUE_FIRSTGODOWNSTAIRS;
		} else if (casterPosition.z > tilePosition.z) {
			return RETURNVALUE_FIRSTGOUPSTAIRS;
		}

		if (const Player* player = caster->getPlayer()) {
			if (player->hasFlag(PlayerFlag_IgnoreProtectionZone)) {
				return RETURNVALUE_NOERROR;
			}
		}
	}

	//pz-zone
	if (aggressive && tile->hasFlag(TILESTATE_PROTECTIONZONE)) {
		return RETURNVALUE_ACTIONNOTPERMITTEDINPROTECTIONZONE;
	}

	return g_events->eventCreatureOnAreaCombat(caster, tile, aggressive);
}

bool Combat::isInPvpZone(const Creature* attacker, const Creature* target)
{
	return attacker->getZone() == ZONE_PVP && target->getZone() == ZONE_PVP;
}

bool Combat::isProtected(const Player* attacker, const Player* target)
{
	uint32_t protectionLevel = g_config.getNumber(ConfigManager::PROTECTION_LEVEL);
	if (target->getLevel() < protectionLevel || attacker->getLevel() < protectionLevel) {
		return true;
	}

	if (target->getHisPokemon()) {
		return true;
	}

	return false;
}

ReturnValue Combat::canDoCombat(Creature* attacker, Creature* target)
{
	if (!attacker) {
		return g_events->eventCreatureOnTargetCombat(attacker, target);
	}

	if (const Player* targetPlayer = target->getPlayer()) {
		if (targetPlayer->hasFlag(PlayerFlag_CannotBeAttacked)) {
			return RETURNVALUE_YOUMAYNOTATTACKTHISPLAYER;
		}

		if (const Player* attackerPlayer = attacker->getPlayer()) {
			if (attackerPlayer->hasFlag(PlayerFlag_CannotAttackPlayer)) {
				return RETURNVALUE_YOUMAYNOTATTACKTHISPLAYER;
			}

			if (isProtected(attackerPlayer, targetPlayer)) {
				return RETURNVALUE_YOUMAYNOTATTACKTHISPLAYER;
			}

			//nopvp-zone
			const Tile* targetPlayerTile = targetPlayer->getTile();
			if (targetPlayerTile->hasFlag(TILESTATE_NOPVPZONE)) {
				return RETURNVALUE_ACTIONNOTPERMITTEDINANOPVPZONE;
			} else if (attackerPlayer->getTile()->hasFlag(TILESTATE_NOPVPZONE) && !targetPlayerTile->hasFlag(TILESTATE_NOPVPZONE | TILESTATE_PROTECTIONZONE)) {
				return RETURNVALUE_ACTIONNOTPERMITTEDINANOPVPZONE;
			}
		}

		if (attacker->isSummon()) {
			if (const Player* masterAttackerPlayer = attacker->getMaster()->getPlayer()) {
				if (masterAttackerPlayer->hasFlag(PlayerFlag_CannotAttackPlayer)) {
					return RETURNVALUE_YOUMAYNOTATTACKTHISPLAYER;
				}

				if (targetPlayer->getTile()->hasFlag(TILESTATE_NOPVPZONE)) {
					return RETURNVALUE_ACTIONNOTPERMITTEDINANOPVPZONE;
				}

				if (isProtected(masterAttackerPlayer, targetPlayer)) {
					return RETURNVALUE_YOUMAYNOTATTACKTHISPLAYER;
				}
			}
		}
	} else if (target->getPokemon()) {
		if (const Player* attackerPlayer = attacker->getPlayer()) {
			if (attackerPlayer->hasFlag(PlayerFlag_CannotAttackPokemon)) {
				return RETURNVALUE_YOUMAYNOTATTACKTHISCREATURE;
			}

			if (target->isSummon() && target->getMaster() == attacker) {
				return RETURNVALUE_YOUMAYNOTATTACKTHISCREATURE;
			}

			if (target->isSummon() && target->getMaster()->getPlayer() && target->getZone() == ZONE_NOPVP) {
				return RETURNVALUE_ACTIONNOTPERMITTEDINANOPVPZONE;
			}
		} else if (attacker->getPokemon()) {
			const Creature* targetMaster = target->getMaster();

			if (targetMaster && targetMaster->getZone() == ZONE_PROTECTION) {
				return RETURNVALUE_YOUMAYNOTATTACKTHISCREATURE;
			}

			if (!targetMaster || !targetMaster->getPlayer()) {
				const Creature* attackerMaster = attacker->getMaster();

				if (!attackerMaster || !attackerMaster->getPlayer()) {
					return RETURNVALUE_YOUMAYNOTATTACKTHISCREATURE;
				}
			}
		}
	}

	if (g_game.getWorldType() == WORLD_TYPE_NO_PVP) {
		if (attacker->getPlayer() || (attacker->isSummon() && attacker->getMaster()->getPlayer())) {
			if (target->getPlayer()) {
				if (!isInPvpZone(attacker, target)) {
					return RETURNVALUE_YOUMAYNOTATTACKTHISPLAYER;
				}
			}

			if (target->isSummon() && target->getMaster()->getPlayer()) {
				if (!isInPvpZone(attacker, target)) {
					return RETURNVALUE_YOUMAYNOTATTACKTHISCREATURE;
				}
			}
		}
	}
	return g_events->eventCreatureOnTargetCombat(attacker, target);
}

void Combat::setPlayerCombatValues(formulaType_t formulaType, double mina, double minb, double maxa, double maxb)
{
	this->formulaType = formulaType;
	this->mina = mina;
	this->minb = minb;
	this->maxa = maxa;
	this->maxb = maxb;
}

bool Combat::setParam(CombatParam_t param, uint32_t value)
{
	switch (param) {
		case COMBAT_PARAM_TYPE: {
			params.combatType = static_cast<CombatType_t>(value);
			return true;
		}

		case COMBAT_PARAM_EFFECT: {
			params.impactEffect = static_cast<uint16_t>(value);
			return true;
		}

		case COMBAT_PARAM_DISTANCEEFFECT: {
			params.distanceEffect = static_cast<uint16_t>(value);
			return true;
		}

		case COMBAT_PARAM_TARGETCASTERORTOPMOST: {
			params.targetCasterOrTopMost = (value != 0);
			return true;
		}

		case COMBAT_PARAM_CREATEITEM: {
			params.itemId = value;
			return true;
		}

		case COMBAT_PARAM_AGGRESSIVE: {
			params.aggressive = (value != 0);
			return true;
		}

		case COMBAT_PARAM_DISPEL: {
			params.dispelType = static_cast<ConditionType_t>(value);
			return true;
		}

		case COMBAT_PARAM_SOUND: {
			params.impactSound = static_cast<uint16_t>(value);
			return true;
		}

		case COMBAT_PARAM_DISTANCESOUND: {
			params.distanceSound = static_cast<uint16_t>(value);
			return true;
		}
	}
	return false;
}

bool Combat::setCallback(CallBackParam_t key)
{
	switch (key) {
		case CALLBACK_PARAM_LEVELATTACKVALUE: {
			params.valueCallback.reset(new ValueCallback(COMBAT_FORMULA_LEVELATTACK));
			return true;
		}

		case CALLBACK_PARAM_LEVELSPECIALATTACKVALUE: {
			params.valueCallback.reset(new ValueCallback(COMBAT_FORMULA_LEVELSPECIALATTACK));
			return true;
		}

		case CALLBACK_PARAM_LEVELDEFENSEVALUE: {
			params.valueCallback.reset(new ValueCallback(COMBAT_FORMULA_LEVELDEFENSE));
			return true;
		}

		case CALLBACK_PARAM_LEVELSPECIALDEFENSEVALUE: {
			params.valueCallback.reset(new ValueCallback(COMBAT_FORMULA_LEVELSPECIALDEFENSE));
			return true;
		}

		case CALLBACK_PARAM_TARGETTILE: {
			params.tileCallback.reset(new TileCallback());
			return true;
		}

		case CALLBACK_PARAM_TARGETCREATURE: {
			params.targetCallback.reset(new TargetCallback());
			return true;
		}
	}
	return false;
}

CallBack* Combat::getCallback(CallBackParam_t key)
{
	switch (key) {
		case CALLBACK_PARAM_LEVELATTACKVALUE:
		case CALLBACK_PARAM_LEVELSPECIALATTACKVALUE:
		case CALLBACK_PARAM_LEVELDEFENSEVALUE: 
		case CALLBACK_PARAM_LEVELSPECIALDEFENSEVALUE: {
			return params.valueCallback.get();
		}

		case CALLBACK_PARAM_TARGETTILE: {
			return params.tileCallback.get();
		}

		case CALLBACK_PARAM_TARGETCREATURE: {
			return params.targetCallback.get();
		}
	}
	return nullptr;
}

void Combat::CombatHealthFunc(Creature* caster, Creature* target, const CombatParams& params, CombatDamage* data)
{
	assert(data);
	CombatDamage damage = *data;
	float modifier = 1.0f;

	if (Pokemon* pokemon = caster->getPokemon()) {
		if (damage.type == pokemonTypeToCombatType(pokemon->getFirstType()) ||
			damage.type == pokemonTypeToCombatType(pokemon->getSecondType())) {
			modifier = 1.5f;
		}
	}

	modifier *= (uniform_random(85, 100) / 100.0);

	float extra = -2.0f;
	if (damage.value > 0) {
		extra *= -1;
	}

	if (damage.stat == STAT_ATTACK) {
		damage.value = (((damage.value * (caster->getAttack() / target->getDefense())) / 50.0f) + extra) * modifier;
	} else if (damage.stat == STAT_SPECIALATTACK) {
		damage.value = (((damage.value * (caster->getSpecialAttack() / target->getSpecialDefense())) / 50.0f) + extra) * modifier;
	} else {
		if (damage.value != 0)
			damage.value = ((damage.value / 50.0f) + extra) * modifier;
	}

	if (g_game.combatBlockHit(damage, caster, target, params.itemId != 0)) {
		return;
	}

	if (g_game.combatChangeHealth(caster, target, damage)) {
		CombatConditionFunc(caster, target, params, &damage);
		CombatDispelFunc(caster, target, params, nullptr);
	}
}

void Combat::CombatConditionFunc(Creature* caster, Creature* target, const CombatParams& params, CombatDamage* data)
{
	if (data && data->value == 0) {
		return;
	}

	for (const auto& condition : params.conditionList) {
		if (caster == target || !target->isImmune(condition->getType())) {
			Condition* conditionCopy = condition->clone();
			if (caster) {
				conditionCopy->setParam(CONDITION_PARAM_OWNER, caster->getID());
			}

			//TODO: infight condition until all aggressive conditions has ended
			target->addCombatCondition(conditionCopy);
		} else {
			g_game.addAnimatedText(target->getPosition(), 215, "IMMUNE");
		}
	}
}

void Combat::CombatDispelFunc(Creature*, Creature* target, const CombatParams& params, CombatDamage*)
{
	target->removeCombatCondition(params.dispelType);
}

void Combat::CombatNullFunc(Creature* caster, Creature* target, const CombatParams& params, CombatDamage*)
{
	CombatConditionFunc(caster, target, params, nullptr);
	CombatDispelFunc(caster, target, params, nullptr);
}

void Combat::combatTileEffects(const SpectatorHashSet& spectators, Creature* caster, Tile* tile, const CombatParams& params)
{
	if (params.itemId != 0) {
		uint16_t itemId = params.itemId;
		switch (itemId) {
			case ITEM_FIREFIELD_PERSISTENT_FULL:
				itemId = ITEM_FIREFIELD_PVP_FULL;
				break;

			case ITEM_FIREFIELD_PERSISTENT_MEDIUM:
				itemId = ITEM_FIREFIELD_PVP_MEDIUM;
				break;

			case ITEM_FIREFIELD_PERSISTENT_SMALL:
				itemId = ITEM_FIREFIELD_PVP_SMALL;
				break;

			case ITEM_ENERGYFIELD_PERSISTENT:
				itemId = ITEM_ENERGYFIELD_PVP;
				break;

			case ITEM_POISONFIELD_PERSISTENT:
				itemId = ITEM_POISONFIELD_PVP;
				break;

			case ITEM_MAGICWALL_PERSISTENT:
				itemId = ITEM_MAGICWALL;
				break;

			case ITEM_WILDGROWTH_PERSISTENT:
				itemId = ITEM_WILDGROWTH;
				break;

			default:
				break;
		}

		if (caster) {
			Player* casterPlayer;
			if (caster->isSummon()) {
				casterPlayer = caster->getMaster()->getPlayer();
			} else {
				casterPlayer = caster->getPlayer();
			}

			if (casterPlayer) {
				if (g_game.getWorldType() == WORLD_TYPE_NO_PVP || tile->hasFlag(TILESTATE_NOPVPZONE)) {
					if (itemId == ITEM_FIREFIELD_PVP_FULL) {
						itemId = ITEM_FIREFIELD_NOPVP;
					} else if (itemId == ITEM_POISONFIELD_PVP) {
						itemId = ITEM_POISONFIELD_NOPVP;
					} else if (itemId == ITEM_ENERGYFIELD_PVP) {
						itemId = ITEM_ENERGYFIELD_NOPVP;
					}
				} else if (itemId == ITEM_FIREFIELD_PVP_FULL || itemId == ITEM_POISONFIELD_PVP || itemId == ITEM_ENERGYFIELD_PVP) {
					casterPlayer->addInFightTicks();
				}
			}
		}

		Item* item = Item::CreateItem(itemId);
		if (caster) {
			item->setOwner(caster->getID());
		}

		ReturnValue ret = g_game.internalAddItem(tile, item);
		if (ret == RETURNVALUE_NOERROR) {
			g_game.startDecay(item);
		} else {
			delete item;
		}
	}

	if (params.tileCallback) {
		params.tileCallback->onTileCombat(caster, tile);
	}

	if (params.impactEffect != CONST_ME_NONE) {
		Game::addEffect(spectators, tile->getPosition(), params.impactEffect);
	}
}

void Combat::postCombatSoundEffects(const SpectatorHashSet& spectators, const CombatParams& params)
{
	if (params.impactSound != CONST_SE_NONE) {
		for (Creature* spectator : spectators) {
			if (Player* tmpPlayer = spectator->getPlayer()) {
				tmpPlayer->sendSound(tmpPlayer->getPosition(), params.impactSound);
			}
		}
	}
}

void Combat::postCombatEffects(Creature* caster, const Position& pos, const CombatParams& params)
{
	if (caster) {
		if (params.distanceEffect != CONST_ANI_NONE) {
			addDistanceEffect(caster, caster->getPosition(), pos, params.distanceEffect);
		}

		if (params.distanceSound != CONST_SE_NONE) {
			addDistanceSound(caster, caster->getPosition(), pos, params.distanceSound);
		}
	}
}

void Combat::addDistanceEffect(Creature* caster, const Position& fromPos, const Position& toPos, uint16_t effect)
{
	if (effect != CONST_ANI_NONE) {
		g_game.addDistanceEffect(fromPos, toPos, effect);
	}
}

void Combat::addDistanceSound(Creature* caster, const Position& fromPos, const Position& toPos, uint16_t sound)
{
	if (sound != CONST_SE_NONE) {
		g_game.addDistanceSound(fromPos, toPos, sound);
	}
}

void Combat::CombatFunc(Creature* caster, const Position& pos, const AreaCombat* area, const CombatParams& params, CombatFunction func, CombatDamage* data)
{
	std::forward_list<Tile*> tileList;

	if (caster) {
		getCombatArea(caster->getPosition(), pos, area, tileList);
	} else {
		getCombatArea(pos, pos, area, tileList);
	}

	SpectatorHashSet spectators;
	uint32_t maxX = 0;
	uint32_t maxY = 0;

	//calculate the max viewable range
	for (Tile* tile : tileList) {
		const Position& tilePos = tile->getPosition();

		uint32_t diff = Position::getDistanceX(tilePos, pos);
		if (diff > maxX) {
			maxX = diff;
		}

		diff = Position::getDistanceY(tilePos, pos);
		if (diff > maxY) {
			maxY = diff;
		}
	}

	const int32_t rangeX = maxX + Map::maxViewportX;
	const int32_t rangeY = maxY + Map::maxViewportY;
	g_game.map.getSpectators(spectators, pos, true, true, rangeX, rangeX, rangeY, rangeY);

	postCombatEffects(caster, pos, params);
	postCombatSoundEffects(spectators, params);

	for (Tile* tile : tileList) {
		if (canDoCombat(caster, tile, params.aggressive) != RETURNVALUE_NOERROR) {
			continue;
		}

		combatTileEffects(spectators, caster, tile, params);

		if (CreatureVector* creatures = tile->getCreatures()) {
			const Creature* topCreature = tile->getTopCreature();
			for (Creature* creature : *creatures) {
				if (params.targetCasterOrTopMost) {
					if (caster && caster->getTile() == tile) {
						if (creature != caster) {
							continue;
						}
					} else if (creature != topCreature) {
						continue;
					}
				}

				if (!params.aggressive || (caster != creature && Combat::canDoCombat(caster, creature) == RETURNVALUE_NOERROR)) {
					func(caster, creature, params, data);
					if (params.targetCallback) {
						params.targetCallback->onTargetCombat(caster, creature);
					}

					if (params.targetCasterOrTopMost) {
						break;
					}
				}
			}
		}
	}
}

void Combat::doCombat(Creature* caster, Creature* target) const
{
	//target combat callback function
	if (params.combatType != COMBAT_NONE) {
		CombatDamage damage = getCombatDamage(caster, target);
		doCombatHealth(caster, target, damage, params);
	} else {
		doCombatDefault(caster, target, params);
	}
}

void Combat::doCombat(Creature* caster, const Position& position) const
{
	//area combat callback function
	if (params.combatType != COMBAT_NONE) {
		CombatDamage damage = getCombatDamage(caster, nullptr);
		doCombatHealth(caster, position, area.get(), damage, params);
	} else {
		CombatFunc(caster, position, area.get(), params, CombatNullFunc, nullptr);
	}
}

void Combat::doCombatHealth(Creature* caster, Creature* target, CombatDamage& damage, const CombatParams& params)
{
	bool canCombat = !params.aggressive || (caster != target && Combat::canDoCombat(caster, target) == RETURNVALUE_NOERROR);
	if (caster == target || canCombat) {
		if (params.impactEffect != CONST_ME_NONE) {
			g_game.addEffect(target->getPosition(), params.impactEffect);
		}

		if (params.impactSound != CONST_SE_NONE) {
			g_game.addSound(target->getPosition(), params.impactSound);
		}		
	}

	if (canCombat) {
		if (caster) {
			if (params.distanceEffect != CONST_ANI_NONE) {
				addDistanceEffect(caster, caster->getPosition(), target->getPosition(), params.distanceEffect);
			}

			if (params.distanceSound != CONST_SE_NONE) {
				addDistanceSound(caster, caster->getPosition(), target->getPosition(), params.distanceSound);
			}
		}

		CombatHealthFunc(caster, target, params, &damage);
		if (params.targetCallback) {
			params.targetCallback->onTargetCombat(caster, target);
		}
	}
}

void Combat::doCombatHealth(Creature* caster, const Position& position, const AreaCombat* area, CombatDamage& damage, const CombatParams& params)
{
	CombatFunc(caster, position, area, params, CombatHealthFunc, &damage);
}

void Combat::doCombatCondition(Creature* caster, const Position& position, const AreaCombat* area, const CombatParams& params)
{
	CombatFunc(caster, position, area, params, CombatConditionFunc, nullptr);
}

void Combat::doCombatCondition(Creature* caster, Creature* target, const CombatParams& params)
{
	bool canCombat = !params.aggressive || (caster != target && Combat::canDoCombat(caster, target) == RETURNVALUE_NOERROR);
	if (caster == target || canCombat) {
		if (params.impactEffect != CONST_ME_NONE) {
			g_game.addEffect(target->getPosition(), params.impactEffect);
		}

		if (params.impactSound != CONST_SE_NONE) {
			g_game.addSound(target->getPosition(), params.impactSound);
		}
	}

	if (canCombat) {
		if (caster) {
			if (params.distanceEffect != CONST_ANI_NONE) {
				addDistanceEffect(caster, caster->getPosition(), target->getPosition(), params.distanceEffect);
			}

			if (params.distanceSound != CONST_SE_NONE) {
				addDistanceSound(caster, caster->getPosition(), target->getPosition(), params.distanceSound);
			}
		}

		CombatConditionFunc(caster, target, params, nullptr);
		if (params.targetCallback) {
			params.targetCallback->onTargetCombat(caster, target);
		}
	}
}

void Combat::doCombatDispel(Creature* caster, const Position& position, const AreaCombat* area, const CombatParams& params)
{
	CombatFunc(caster, position, area, params, CombatDispelFunc, nullptr);
}

void Combat::doCombatDispel(Creature* caster, Creature* target, const CombatParams& params)
{
	bool canCombat = !params.aggressive || (caster != target && Combat::canDoCombat(caster, target) == RETURNVALUE_NOERROR);
	if (caster == target || canCombat) {
		if (params.impactEffect != CONST_ME_NONE) {
			g_game.addEffect(target->getPosition(), params.impactEffect);
		}

		if (params.impactSound != CONST_ME_NONE) {
			g_game.addSound(target->getPosition(), params.impactSound);
		}
	}

	if (canCombat) {
		CombatDispelFunc(caster, target, params, nullptr);
		if (params.targetCallback) {
			params.targetCallback->onTargetCombat(caster, target);
		}

		if (caster) {
			if (params.distanceEffect != CONST_ANI_NONE) {
				addDistanceEffect(caster, caster->getPosition(), target->getPosition(), params.distanceEffect);
			}

			if (params.distanceSound != CONST_SE_NONE) {
				addDistanceSound(caster, caster->getPosition(), target->getPosition(), params.distanceSound);
			}
		}
	}
}

void Combat::doCombatDefault(Creature* caster, Creature* target, const CombatParams& params)
{
	if (!params.aggressive || (caster != target && Combat::canDoCombat(caster, target) == RETURNVALUE_NOERROR)) {
		SpectatorHashSet spectators;
		g_game.map.getSpectators(spectators, target->getPosition(), true, true);

		CombatNullFunc(caster, target, params, nullptr);
		combatTileEffects(spectators, caster, target->getTile(), params);

		if (params.targetCallback) {
			params.targetCallback->onTargetCombat(caster, target);
		}

		if (params.impactEffect != CONST_ME_NONE) {
			g_game.addEffect(target->getPosition(), params.impactEffect);
		}

		if (caster) {
			if (params.distanceEffect != CONST_ANI_NONE) {
				addDistanceEffect(caster, caster->getPosition(), target->getPosition(), params.distanceEffect);
			}

			if (params.distanceSound != CONST_SE_NONE) {
				addDistanceSound(caster, caster->getPosition(), target->getPosition(), params.distanceSound);
			}
		}
	}
}

//**********************************************************//

void ValueCallback::getDamageValue(Pokemon* pokemon, CombatDamage& damage) const
{
	//onGetPokemonMinMaxValues(...)
	if (!scriptInterface->reserveScriptEnv()) {
		std::cout << "[Error - ValueCallback::getDamageValue] Call stack overflow" << std::endl;
		return;
	}

	ScriptEnvironment* env = scriptInterface->getScriptEnv();
	if (!env->setCallbackId(scriptId, scriptInterface)) {
		scriptInterface->resetScriptEnv();
		return;
	}

	lua_State* L = scriptInterface->getLuaState();

	scriptInterface->pushFunction(scriptId);

	LuaScriptInterface::pushUserdata<Pokemon>(L, pokemon);
	LuaScriptInterface::setMetatable(L, -1, "Pokemon");

	int parameters = 1;
	switch (type) {
		case COMBAT_FORMULA_LEVELATTACK: {
			//onGetPokemonDamageValue(pokemon, level)
			lua_pushnumber(L, pokemon->getLevel());

			parameters += 1;
			damage.stat = STAT_ATTACK;
			break;
		}
		case COMBAT_FORMULA_LEVELSPECIALATTACK: {
			//onGetPokemonDamageValue(pokemon, level)
			lua_pushnumber(L, pokemon->getLevel());

			parameters += 1;
			damage.stat = STAT_SPECIALATTACK;
			break;
		}
		case COMBAT_FORMULA_LEVELDEFENSE: {
			//onGetPokemonDamageValue(pokemon, level, def)
			lua_pushnumber(L, pokemon->getLevel());
			lua_pushnumber(L, pokemon->getDefense());

			parameters += 2;
			damage.stat = STAT_DEFENSE;
			break;
		}
		case COMBAT_FORMULA_LEVELSPECIALDEFENSE: {
			//onGetPokemonDamageValue(pokemon, level, spdef)
			lua_pushnumber(L, pokemon->getLevel());
			lua_pushnumber(L, pokemon->getSpecialDefense());

			parameters += 2;
			damage.stat = STAT_SPECIALDEFENSE;
			break;
		}

		default: {
			std::cout << "ValueCallback::getDamageValue - unknown callback type" << std::endl;
			scriptInterface->resetScriptEnv();
			return;
		}
	}

	int size0 = lua_gettop(L);
	if (lua_pcall(L, parameters, 1, 0) != 0) {
		LuaScriptInterface::reportError(nullptr, LuaScriptInterface::popString(L));
	} else {
		damage.value = LuaScriptInterface::getNumber<int32_t>(L, -1);
		lua_pop(L, 1);
	}

	if ((lua_gettop(L) + parameters + 1) != size0) {
		LuaScriptInterface::reportError(nullptr, "Stack size changed!");
	}

	scriptInterface->resetScriptEnv();
}

//**********************************************************//

void TileCallback::onTileCombat(Creature* creature, Tile* tile) const
{
	//onTileCombat(creature, pos)
	if (!scriptInterface->reserveScriptEnv()) {
		std::cout << "[Error - TileCallback::onTileCombat] Call stack overflow" << std::endl;
		return;
	}

	ScriptEnvironment* env = scriptInterface->getScriptEnv();
	if (!env->setCallbackId(scriptId, scriptInterface)) {
		scriptInterface->resetScriptEnv();
		return;
	}

	lua_State* L = scriptInterface->getLuaState();

	scriptInterface->pushFunction(scriptId);
	if (creature) {
		LuaScriptInterface::pushUserdata<Creature>(L, creature);
		LuaScriptInterface::setCreatureMetatable(L, -1, creature);
	} else {
		lua_pushnil(L);
	}
	LuaScriptInterface::pushPosition(L, tile->getPosition());

	scriptInterface->callFunction(2);
}

//**********************************************************//

void TargetCallback::onTargetCombat(Creature* creature, Creature* target) const
{
	//onTargetCombat(creature, target)
	if (!scriptInterface->reserveScriptEnv()) {
		std::cout << "[Error - TargetCallback::onTargetCombat] Call stack overflow" << std::endl;
		return;
	}

	ScriptEnvironment* env = scriptInterface->getScriptEnv();
	if (!env->setCallbackId(scriptId, scriptInterface)) {
		scriptInterface->resetScriptEnv();
		return;
	}

	lua_State* L = scriptInterface->getLuaState();

	scriptInterface->pushFunction(scriptId);

	if (creature) {
		LuaScriptInterface::pushUserdata<Creature>(L, creature);
		LuaScriptInterface::setCreatureMetatable(L, -1, creature);
	} else {
		lua_pushnil(L);
	}

	if (target) {
		LuaScriptInterface::pushUserdata<Creature>(L, target);
		LuaScriptInterface::setCreatureMetatable(L, -1, target);
	} else {
		lua_pushnil(L);
	}

	int size0 = lua_gettop(L);

	if (lua_pcall(L, 2, 0 /*nReturnValues*/, 0) != 0) {
		LuaScriptInterface::reportError(nullptr, LuaScriptInterface::popString(L));
	}

	if ((lua_gettop(L) + 2 /*nParams*/ + 1) != size0) {
		LuaScriptInterface::reportError(nullptr, "Stack size changed!");
	}

	scriptInterface->resetScriptEnv();
}

//**********************************************************//

void AreaCombat::clear()
{
	for (const auto& it : areas) {
		delete it.second;
	}
	areas.clear();
}

AreaCombat::AreaCombat(const AreaCombat& rhs)
{
	hasExtArea = rhs.hasExtArea;
	for (const auto& it : rhs.areas) {
		areas[it.first] = new MatrixArea(*it.second);
	}
}

void AreaCombat::getList(const Position& centerPos, const Position& targetPos, std::forward_list<Tile*>& list) const
{
	const MatrixArea* area = getArea(centerPos, targetPos);
	if (!area) {
		return;
	}

	uint32_t centerY, centerX;
	area->getCenter(centerY, centerX);

	Position tmpPos(targetPos.x - centerX, targetPos.y - centerY, targetPos.z);
	uint32_t cols = area->getCols();
	for (uint32_t y = 0, rows = area->getRows(); y < rows; ++y) {
		for (uint32_t x = 0; x < cols; ++x) {
			if (area->getValue(y, x) != 0) {
				if (g_game.isSightClear(targetPos, tmpPos, true)) {
					Tile* tile = g_game.map.getTile(tmpPos);
					if (!tile) {
						tile = new StaticTile(tmpPos.x, tmpPos.y, tmpPos.z);
						g_game.map.setTile(tmpPos, tile);
					}
					list.push_front(tile);
				}
			}
			tmpPos.x++;
		}
		tmpPos.x -= cols;
		tmpPos.y++;
	}
}

void AreaCombat::copyArea(const MatrixArea* input, MatrixArea* output, MatrixOperation_t op) const
{
	uint32_t centerY, centerX;
	input->getCenter(centerY, centerX);

	if (op == MATRIXOPERATION_COPY) {
		for (uint32_t y = 0; y < input->getRows(); ++y) {
			for (uint32_t x = 0; x < input->getCols(); ++x) {
				(*output)[y][x] = (*input)[y][x];
			}
		}

		output->setCenter(centerY, centerX);
	} else if (op == MATRIXOPERATION_MIRROR) {
		for (uint32_t y = 0; y < input->getRows(); ++y) {
			uint32_t rx = 0;
			for (int32_t x = input->getCols(); --x >= 0;) {
				(*output)[y][rx++] = (*input)[y][x];
			}
		}

		output->setCenter(centerY, (input->getRows() - 1) - centerX);
	} else if (op == MATRIXOPERATION_FLIP) {
		for (uint32_t x = 0; x < input->getCols(); ++x) {
			uint32_t ry = 0;
			for (int32_t y = input->getRows(); --y >= 0;) {
				(*output)[ry++][x] = (*input)[y][x];
			}
		}

		output->setCenter((input->getCols() - 1) - centerY, centerX);
	} else {
		// rotation
		int32_t rotateCenterX = (output->getCols() / 2) - 1;
		int32_t rotateCenterY = (output->getRows() / 2) - 1;
		int32_t angle;

		switch (op) {
			case MATRIXOPERATION_ROTATE90:
				angle = 90;
				break;

			case MATRIXOPERATION_ROTATE180:
				angle = 180;
				break;

			case MATRIXOPERATION_ROTATE270:
				angle = 270;
				break;

			default:
				angle = 0;
				break;
		}

		double angleRad = M_PI * angle / 180.0;

		double a = std::cos(angleRad);
		double b = -std::sin(angleRad);
		double c = std::sin(angleRad);
		double d = std::cos(angleRad);

		const uint32_t rows = input->getRows();
		for (uint32_t x = 0, cols = input->getCols(); x < cols; ++x) {
			for (uint32_t y = 0; y < rows; ++y) {
				//calculate new coordinates using rotation center
				int32_t newX = x - centerX;
				int32_t newY = y - centerY;

				//perform rotation
				int32_t rotatedX = static_cast<int32_t>(round(newX * a + newY * b));
				int32_t rotatedY = static_cast<int32_t>(round(newX * c + newY * d));

				//write in the output matrix using rotated coordinates
				(*output)[rotatedY + rotateCenterY][rotatedX + rotateCenterX] = (*input)[y][x];
			}
		}

		output->setCenter(rotateCenterY, rotateCenterX);
	}
}

MatrixArea* AreaCombat::createArea(const std::list<uint32_t>& list, uint32_t rows)
{
	uint32_t cols;
	if (rows == 0) {
		cols = 0;
	} else {
		cols = list.size() / rows;
	}

	MatrixArea* area = new MatrixArea(rows, cols);

	uint32_t x = 0;
	uint32_t y = 0;

	for (uint32_t value : list) {
		if (value == 1 || value == 3) {
			area->setValue(y, x, true);
		}

		if (value == 2 || value == 3) {
			area->setCenter(y, x);
		}

		++x;

		if (cols == x) {
			x = 0;
			++y;
		}
	}
	return area;
}

void AreaCombat::setupArea(const std::list<uint32_t>& list, uint32_t rows)
{
	MatrixArea* area = createArea(list, rows);

	//NORTH
	areas[DIRECTION_NORTH] = area;

	uint32_t maxOutput = std::max<uint32_t>(area->getCols(), area->getRows()) * 2;

	//SOUTH
	MatrixArea* southArea = new MatrixArea(maxOutput, maxOutput);
	copyArea(area, southArea, MATRIXOPERATION_ROTATE180);
	areas[DIRECTION_SOUTH] = southArea;

	//EAST
	MatrixArea* eastArea = new MatrixArea(maxOutput, maxOutput);
	copyArea(area, eastArea, MATRIXOPERATION_ROTATE90);
	areas[DIRECTION_EAST] = eastArea;

	//WEST
	MatrixArea* westArea = new MatrixArea(maxOutput, maxOutput);
	copyArea(area, westArea, MATRIXOPERATION_ROTATE270);
	areas[DIRECTION_WEST] = westArea;
}

void AreaCombat::setupArea(int32_t length, int32_t spread)
{
	std::list<uint32_t> list;

	uint32_t rows = length;
	int32_t cols = 1;

	if (spread != 0) {
		cols = ((length - (length % spread)) / spread) * 2 + 1;
	}

	int32_t colSpread = cols;

	for (uint32_t y = 1; y <= rows; ++y) {
		int32_t mincol = cols - colSpread + 1;
		int32_t maxcol = cols - (cols - colSpread);

		for (int32_t x = 1; x <= cols; ++x) {
			if (y == rows && x == ((cols - (cols % 2)) / 2) + 1) {
				list.push_back(3);
			} else if (x >= mincol && x <= maxcol) {
				list.push_back(1);
			} else {
				list.push_back(0);
			}
		}

		if (spread > 0 && y % spread == 0) {
			--colSpread;
		}
	}

	setupArea(list, rows);
}

void AreaCombat::setupArea(int32_t radius)
{
	int32_t area[13][13] = {
		{0, 0, 0, 0, 0, 0, 8, 0, 0, 0, 0, 0, 0},
		{0, 0, 0, 0, 8, 8, 7, 8, 8, 0, 0, 0, 0},
		{0, 0, 0, 8, 7, 6, 6, 6, 7, 8, 0, 0, 0},
		{0, 0, 8, 7, 6, 5, 5, 5, 6, 7, 8, 0, 0},
		{0, 8, 7, 6, 5, 4, 4, 4, 5, 6, 7, 8, 0},
		{0, 8, 6, 5, 4, 3, 2, 3, 4, 5, 6, 8, 0},
		{8, 7, 6, 5, 4, 2, 1, 2, 4, 5, 6, 7, 8},
		{0, 8, 6, 5, 4, 3, 2, 3, 4, 5, 6, 8, 0},
		{0, 8, 7, 6, 5, 4, 4, 4, 5, 6, 7, 8, 0},
		{0, 0, 8, 7, 6, 5, 5, 5, 6, 7, 8, 0, 0},
		{0, 0, 0, 8, 7, 6, 6, 6, 7, 8, 0, 0, 0},
		{0, 0, 0, 0, 8, 8, 7, 8, 8, 0, 0, 0, 0},
		{0, 0, 0, 0, 0, 0, 8, 0, 0, 0, 0, 0, 0}
	};

	std::list<uint32_t> list;

	for (auto& row : area) {
		for (int cell : row) {
			if (cell == 1) {
				list.push_back(3);
			} else if (cell > 0 && cell <= radius) {
				list.push_back(1);
			} else {
				list.push_back(0);
			}
		}
	}

	setupArea(list, 13);
}

void AreaCombat::setupExtArea(const std::list<uint32_t>& list, uint32_t rows)
{
	if (list.empty()) {
		return;
	}

	hasExtArea = true;
	MatrixArea* area = createArea(list, rows);

	//NORTH-WEST
	areas[DIRECTION_NORTHWEST] = area;

	uint32_t maxOutput = std::max<uint32_t>(area->getCols(), area->getRows()) * 2;

	//NORTH-EAST
	MatrixArea* neArea = new MatrixArea(maxOutput, maxOutput);
	copyArea(area, neArea, MATRIXOPERATION_MIRROR);
	areas[DIRECTION_NORTHEAST] = neArea;

	//SOUTH-WEST
	MatrixArea* swArea = new MatrixArea(maxOutput, maxOutput);
	copyArea(area, swArea, MATRIXOPERATION_FLIP);
	areas[DIRECTION_SOUTHWEST] = swArea;

	//SOUTH-EAST
	MatrixArea* seArea = new MatrixArea(maxOutput, maxOutput);
	copyArea(swArea, seArea, MATRIXOPERATION_MIRROR);
	areas[DIRECTION_SOUTHEAST] = seArea;
}

//**********************************************************//

void MagicField::onStepInField(Creature* creature)
{
	//remove magic walls/wild growth
	if (id == ITEM_MAGICWALL || id == ITEM_WILDGROWTH || id == ITEM_MAGICWALL_SAFE || id == ITEM_WILDGROWTH_SAFE || isBlocking()) {
		if (!creature->isInGhostMode()) {
			g_game.internalRemoveItem(this, 1);
		}

		return;
	}

	const ItemType& it = items[getID()];
	if (it.conditionDamage) {
		Condition* conditionCopy = it.conditionDamage->clone();
		uint32_t ownerId = getOwner();
		if (ownerId) {
			bool harmfulField = true;

			if (g_game.getWorldType() == WORLD_TYPE_NO_PVP || getTile()->hasFlag(TILESTATE_NOPVPZONE)) {
				Creature* owner = g_game.getCreatureByID(ownerId);
				if (owner) {
					if (owner->getPlayer() || (owner->isSummon() && owner->getMaster()->getPlayer())) {
						harmfulField = false;
					}
				}
			}

			Player* targetPlayer = creature->getPlayer();
			if (targetPlayer) {
				Player* attackerPlayer = g_game.getPlayerByID(ownerId);
				if (attackerPlayer) {
					if (Combat::isProtected(attackerPlayer, targetPlayer)) {
						harmfulField = false;
					}
				}
			}

			if (!harmfulField || (OTSYS_TIME() - createTime <= 5000) || creature->hasBeenAttacked(ownerId)) {
				conditionCopy->setParam(CONDITION_PARAM_OWNER, ownerId);
			}
		}

		creature->addCondition(conditionCopy);
	}
}
