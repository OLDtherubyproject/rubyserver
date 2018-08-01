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

#ifndef FS_POKEBALLS_H_69D1993478AA42948E24C0B90B8F5BF5
#define FS_POKEBALLS_H_69D1993478AA42948E24C0B90B8F5BF5

#include "luascript.h"
#include "player.h"
#include "baseevents.h"
#include "combat.h"
#include "const.h"

class PokeballType;

class Pokeballs final : public BaseEvents
{
	public:
		Pokeballs();
		~Pokeballs();

		// non-copyable
		Pokeballs(const Pokeballs&) = delete;
		Pokeballs& operator=(const Pokeballs&) = delete;

		PokeballType* getPokeballTypeByItemID(const uint16_t id) const;
		PokeballType* getPokeballTypeByServerID(const uint16_t id) const;
		PokeballType* getPokeballTypeByName(const std::string& name) const;

		void usePokeball(Player* player, Item* pokeball, const Position& fromPos, const Position& toPos, bool isHotkey) const;
	private:
		void clear() final;
		LuaScriptInterface& getScriptInterface() override;
		std::string getScriptBaseName() const override;
		std::string getScriptPrefixName() const override;
		Event_ptr getEvent(const std::string& nodeName) override;
		bool registerEvent(Event_ptr event, const pugi::xml_node& node) override;

		std::map<uint32_t, PokeballType*> pokeballs;

		LuaScriptInterface scriptInterface { "Pokeball Interface" };
};

class PokeballType : public Event
{
	public:
		explicit PokeballType(LuaScriptInterface* interface) : Event(interface) {}

		bool configureEvent(const pugi::xml_node& node) override;
		bool loadFunction(const pugi::xml_attribute&) override final {
			return true;
		}

		ReturnValue playerPokeballCheck(Player* player, const Position&) const;
		virtual bool usePokeball(Player* player, Item* pokeball, const Position& fromPos, const Position& toPos, bool isHotkey) const;

		uint16_t getServerID() const {
			return serverid;
		}
		uint16_t getItemId() const {
			return itemid;
		}

		std::string getName() const {
			return name;
		}

		uint32_t getReqLevel() const {
			return level;
		}

		double getRate() const {
			return rate;
		}

		uint16_t getChargedID() const {
			return charged;
		}

		uint16_t getDischargedID() const {
			return discharged;
		}

		EffectClasses getGobackEffect() const {
			return static_cast<EffectClasses>(goback);
		}

		uint16_t getCatchSuccessEffect() const {
			return catchSuccess;
		}

		uint16_t getCatchFailEffect() const {
			return catchFail;
		}

		ShootType_t getShotEffect() const {
			return static_cast<ShootType_t>(shotEffect);
		}

		bool isPremium() const {
			return premium;
		}

	protected:
		void internalUsePokeball(Player* player, Item* item, const Position& fromPos, Item* corpse, 
		                          const Position& toPos, bool isHotkey) const;

		CombatParams params;

		uint16_t serverid = 0;
		uint16_t itemid = 0;

	private:
		std::string name;

		uint32_t level = 0;
		uint16_t charged = 0;
		uint16_t discharged = 0;
		uint16_t goback = 0;
		uint16_t catchSuccess = 0;
		uint16_t catchFail = 0;
		uint16_t shotEffect = 0;

		double rate = 0;

		bool premium = false;

	private:
		std::string getScriptEventName() const override final;

		double executeUsePokeball(Player* player, Item* pokeball, const Position& fromPos, Item* corpse, const Position& toPos, bool isHotkey) const;

		static void decrementItemCount(Item* item);

		friend class Combat;
};

#endif