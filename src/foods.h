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

#ifndef FS_FOODS_H_28374B62387R6CQB287R6BQ874R6USYF
#define FS_FOODS_H_28374B62387R6CQB287R6BQ874R6USYF

#include "luascript.h"
#include "player.h"
#include "baseevents.h"
#include "combat.h"
#include "const.h"

class FoodType;

class Foods final : public BaseEvents
{
	public:
		Foods();
		~Foods();

		// non-copyable
		Foods(const Foods&) = delete;
		Foods& operator=(const Foods&) = delete;

		FoodType* getFoodTypeByItemID(const uint16_t id) const;
		FoodType* getFoodTypeByName(const std::string& name) const;

		void useFood(Player* player, const Position& fromPos, const Position& toPos, Item* item, bool isHotkey, Creature* creature = nullptr) const;
	private:
		void clear() final;
		LuaScriptInterface& getScriptInterface() override;
		std::string getScriptBaseName() const override;
		std::string getScriptPrefixName() const override;
		Event_ptr getEvent(const std::string& nodeName) override;
		bool registerEvent(Event_ptr event, const pugi::xml_node& node) override;

		std::map<uint32_t, FoodType*> foods;

		LuaScriptInterface scriptInterface { "Food Interface" };
};

class FoodType : public Event
{
	public:
		explicit FoodType(LuaScriptInterface* interface) : Event(interface) {}

		bool configureEvent(const pugi::xml_node& node) override;
		bool loadFunction(const pugi::xml_attribute&) override final {
			return true;
		}

		ReturnValue playerFoodCheck(Player* player, const Position& toPos, Creature* creature = nullptr) const;
		virtual bool useFood(Player* player, const Position& fromPos, const Position& toPos, Item* item, bool isHotkey, Creature* creature = nullptr) const;

		const ItemType& getItemType() const {
			return Item::items[itemid];
		}

		uint16_t getItemId() const {
			return itemid;
		}
		uint16_t getRegen() const {
			return regen;
		}

		uint32_t getReqLevel() const {
			return level;
		}

		bool isPremium() const {
			return premium;
		}
		bool canEveryoneEat() const {
			return (targets == 3);
		}
		bool canPokemonEat() const {
			return (targets == 2 || targets == 3);
		}
		bool canPlayerEat() const {
			return (targets == 1 || targets == 3);
		}

		std::string getSound() const {
			return sound;
		}

		bool getNatureLikes(Natures_t nature) const;
	protected:
		void internalUseFood(Player* player, const Position& fromPos, const Position& toPos, 
		                        Item* item, bool isHotkey, Creature* creature = nullptr) const;

		uint16_t itemid = 0;

	private:
		std::string sound = "Mmmm.";

		uint8_t targets = 3;
		uint16_t regen = 0;
		uint32_t level = 0;

		bool premium = false;
		bool natureLikes[NATURE_LAST + 1] = {false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false};

	private:
		std::string getScriptEventName() const override final;

		bool executeUseFood(Player* player, const Position& fromPos, const Position& toPos, Item* item, bool isHotkey, Creature* creature = nullptr) const;

		static void decrementItemCount(Item* item);

		friend class Combat;
};

#endif