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

#ifndef FS_MOVES_H_D78A7CCB7080406E8CAA6B1D31D3DA71
#define FS_MOVES_H_D78A7CCB7080406E8CAA6B1D31D3DA71

#include "luascript.h"
#include "player.h"
#include "actions.h"
#include "talkaction.h"
#include "baseevents.h"

class Move;

class Moves final : public BaseEvents
{
	public:
		Moves();
		~Moves();

		// non-copyable
		Moves(const Moves&) = delete;
		Moves& operator=(const Moves&) = delete;

		Move* getMove(uint16_t moveId);
		Move* getMoveByName(const std::string& name);

		static Position getCasterPosition(Creature* creature, Direction dir);
		std::string getScriptBaseName() const override;
		std::string getScriptPrefixName() const override;

		const std::map<std::string, Move>& getMoves() const {
			return moves;
		};

		PokemonType_t MoveTypeToPokemonType(MoveType_t type);
		MoveType_t PokemonTypeToMoveType(PokemonType_t type);

	private:
		void clear() override;
		LuaScriptInterface& getScriptInterface() override;
		Event_ptr getEvent(const std::string& nodeName) override;
		bool registerEvent(Event_ptr event, const pugi::xml_node& node) override;

		std::map<std::string, Move> moves;

		LuaScriptInterface scriptInterface { "Move Interface" };
};

class Move final : public TalkAction
{
	public:
		explicit Move(LuaScriptInterface* interface) : TalkAction(interface) {}

		bool configureEvent(const pugi::xml_node& node) override;
		bool configureMove(const pugi::xml_node& node);
		const std::string& getName() const {
			return name;
		}

		bool castMove(Creature* creature);
		bool castMove(Creature* creature, Creature* target);

		uint16_t getId() const {
			return moveId;
		}
		uint32_t getLevel() const {
			return level;
		}
		uint32_t getCooldown() const {
			return cooldown;
		}
		uint16_t getAccuracy() const {
			return accuracy;
		}
		MoveType_t getType() const {
			return type;
		}
		bool isPremium() const {
			return premium;
		}
		bool isAggressive() const {
			return aggressive;
		}
		bool getHasParam() const {
			return hasParam;
		}
		bool getHasPlayerNameParam() const {
			return hasPlayerNameParam;
		}
		void setType(MoveType_t newType) {
			type = newType;
		}
		bool isPersistent() const {
			return persistent;
		}
		bool executeCastMove(Creature* creature, const LuaVariant& var);
		bool canCast(const Player* player) const;
		bool canThrowMove(const Creature* creature, const Creature* target) const;
		void serialize(PropWriteStream& propWriteStream);

		bool needTarget = false;
		bool ignoreSleep = false;

	protected:
		MoveCategory_t category = MOVECATEGORY_PHYSICAL;
		MoveType_t type = MOVETYPE_NONE;

		uint32_t cooldown = 1000;
		uint32_t level = 0;
		uint32_t accuracy = 100;
		int32_t range = -1;

		uint16_t moveId = 0;

		bool selfTarget = false;

	private:
		bool needDirection = false;
		bool hasParam = false;
		bool hasPlayerNameParam = false;
		bool checkLineOfSight = true;
		bool casterTargetOrDirection = false;
		bool blockingSolid = false;
		bool blockingCreature = false;
		bool aggressive = true;
		bool enabled = true;
		bool premium = false;
		bool persistent = true;

		std::string name;
		std::string getScriptEventName() const override;

		bool internalCastMove(Creature* creature, const LuaVariant& var);
};

#endif
