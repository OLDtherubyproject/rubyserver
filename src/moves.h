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

class InstantMove;
class RuneMove;
class Move;

using VocMoveMap = std::map<uint16_t, bool>;

class Moves final : public BaseEvents
{
	public:
		Moves();
		~Moves();

		// non-copyable
		Moves(const Moves&) = delete;
		Moves& operator=(const Moves&) = delete;

		Move* getMoveByName(const std::string& name);
		RuneMove* getRuneMove(uint32_t id);
		RuneMove* getRuneMoveByName(const std::string& name);

		InstantMove* getInstantMove(const std::string& words);
		InstantMove* getInstantMoveByName(const std::string& name);

		InstantMove* getInstantMoveById(uint32_t moveId);

		static Position getCasterPosition(Creature* creature, Direction dir);
		std::string getScriptBaseName() const override;

		const std::map<std::string, InstantMove>& getInstantMoves() const {
			return instants;
		};

	private:
		void clear() override;
		LuaScriptInterface& getScriptInterface() override;
		Event_ptr getEvent(const std::string& nodeName) override;
		bool registerEvent(Event_ptr event, const pugi::xml_node& node) override;

		std::map<uint16_t, RuneMove> runes;
		std::map<std::string, InstantMove> instants;

		friend class CombatMove;
		LuaScriptInterface scriptInterface { "Move Interface" };
};

using RuneMoveFunction = std::function<bool(const RuneMove* move, Player* player, const Position& posTo)>;

class BaseMove
{
	public:
		constexpr BaseMove() = default;
		virtual ~BaseMove() = default;

		virtual bool castMove(Creature* creature) = 0;
		virtual bool castMove(Creature* creature, Creature* target) = 0;
};

class CombatMove final : public Event, public BaseMove
{
	public:
		CombatMove(Combat* combat, bool needTarget, bool needDirection);
		~CombatMove();

		// non-copyable
		CombatMove(const CombatMove&) = delete;
		CombatMove& operator=(const CombatMove&) = delete;

		bool castMove(Creature* creature) override;
		bool castMove(Creature* creature, Creature* target) override;
		bool configureEvent(const pugi::xml_node&) override {
			return true;
		}

		//scripting
		bool executeCastMove(Creature* creature, const LuaVariant& var);

		bool loadScriptCombat();
		Combat* getCombat() {
			return combat;
		}

	private:
		std::string getScriptEventName() const override {
			return "onCastMove";
		}

		Combat* combat;

		bool needDirection;
		bool needTarget;
};

class Move : public BaseMove
{
	public:
		Move() = default;

		bool configureMove(const pugi::xml_node& node);
		const std::string& getName() const {
			return name;
		}

		void postCastMove(Player* player, bool finishedCast = true, bool payCost = true) const;
		static void postCastMove(Player* player, uint32_t manaCost);

		uint32_t getManaCost(const Player* player) const;
		uint32_t getLevel() const {
			return level;
		}
		uint32_t getMagicLevel() const {
			return magLevel;
		}
		uint32_t getPokemonHealth() const {
			return mana;
		}
		uint32_t getManaPercent() const {
			return manaPercent;
		}
		bool isPremium() const {
			return premium;
		}

		virtual bool isInstant() const = 0;
		bool isLearnable() const {
			return learnable;
		}

		const VocMoveMap& getVocMap() const {
			return vocMoveMap;
		}

		bool needTarget = false;

	protected:
		bool playerMoveCheck(Player* player) const;
		bool playerInstantMoveCheck(Player* player, const Position& toPos);
		bool playerRuneMoveCheck(Player* player, const Position& toPos);

		VocMoveMap vocMoveMap;

		MoveGroup_t group = MOVEGROUP_NONE;
		MoveGroup_t secondaryGroup = MOVEGROUP_NONE;

		uint32_t cooldown = 1000;
		uint32_t groupCooldown = 1000;
		uint32_t secondaryGroupCooldown = 0;
		uint32_t level = 0;
		uint32_t magLevel = 0;
		int32_t range = -1;

		uint8_t moveId = 0;

		bool selfTarget = false;

	private:

		uint32_t mana = 0;
		uint32_t manaPercent = 0;

		bool needWeapon = false;
		bool blockingSolid = false;
		bool blockingCreature = false;
		bool aggressive = true;
		bool learnable = false;
		bool enabled = true;
		bool premium = false;


	private:
		std::string name;
};

class InstantMove final : public TalkAction, public Move
{
	public:
		explicit InstantMove(LuaScriptInterface* interface) : TalkAction(interface) {}

		bool configureEvent(const pugi::xml_node& node) override;

		virtual bool playerCastInstant(Player* player, std::string& param);

		bool castMove(Creature* creature) override;
		bool castMove(Creature* creature, Creature* target) override;

		//scripting
		bool executeCastMove(Creature* creature, const LuaVariant& var);

		bool isInstant() const override {
			return true;
		}
		bool getHasParam() const {
			return hasParam;
		}
		bool getHasPlayerNameParam() const {
			return hasPlayerNameParam;
		}
		bool canCast(const Player* player) const;
		bool canThrowMove(const Creature* creature, const Creature* target) const;

	private:
		std::string getScriptEventName() const override;

		bool internalCastMove(Creature* creature, const LuaVariant& var);

		bool needDirection = false;
		bool hasParam = false;
		bool hasPlayerNameParam = false;
		bool checkLineOfSight = true;
		bool casterTargetOrDirection = false;
};

class RuneMove final : public Action, public Move
{
	public:
		explicit RuneMove(LuaScriptInterface* interface) : Action(interface) {}

		bool configureEvent(const pugi::xml_node& node) override;

		ReturnValue canExecuteAction(const Player* player, const Position& toPos) override;
		bool hasOwnErrorHandler() override {
			return true;
		}
		Thing* getTarget(Player*, Creature* targetCreature, const Position&, uint8_t) const override {
			return targetCreature;
		}

		bool executeUse(Player* player, Item* item, const Position& fromPosition, Thing* target, const Position& toPosition, bool isHotkey) override;

		bool castMove(Creature* creature) override;
		bool castMove(Creature* creature, Creature* target) override;

		//scripting
		bool executeCastMove(Creature* creature, const LuaVariant& var, bool isHotkey);

		bool isInstant() const override {
			return false;
		}
		uint16_t getRuneItemId() const {
			return runeId;
		}

	private:
		std::string getScriptEventName() const override;

		bool internalCastMove(Creature* creature, const LuaVariant& var, bool isHotkey);

		uint16_t runeId = 0;
		bool hasCharges = true;
};

#endif
