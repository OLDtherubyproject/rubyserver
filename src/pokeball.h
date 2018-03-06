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

#ifndef FS_POKEBALL_H_ADCAA356C0DB44CEBA994A0D678EC92D
#define FS_POKEBALL_H_ADCAA356C0DB44CEBA994A0D678EC92D

#include "enums.h"
#include "item.h"

class Pokeball
{

	public:
		explicit Pokeball(uint16_t id) : id(id) {}

		uint16_t getServerId() const {
			return serverid;
		}

		uint16_t getId() const {
			return id;
		}

		const std::string& getName() const {
			return name;
		}

		uint16_t getCharged() const {
			return charged;
		}

		uint16_t getDischarged() const {
			return discharged;
		}

		uint16_t getGoback() const {
			return goback;
		}

		uint16_t getCatchSuccess() const {
			return catchSuccess;
		}

		uint16_t getCatchFail() const {
			return catchFail;
		}

		uint16_t getShotEffect() const {
			return shotEffect;
		}

		uint32_t getLevel() const {
			return level;
		}

		uint8_t getDefaultMultiplier() const {
			return rates.all;
		}

		Rates_t getRates() const {
			return rates;
		}

	private:
		friend class Pokeballs;

		std::string name = "none";

		uint16_t serverid;
		uint16_t id;
        uint16_t charged;
        uint16_t discharged;
        uint16_t goback;
        uint16_t catchSuccess;
        uint16_t catchFail;
		uint16_t shotEffect;
		uint32_t level = 0;

        Rates_t rates = {};
};

class Pokeballs
{
	public:
		bool loadFromXml();

		Pokeball* getPokeball(uint16_t id);
		Pokeball* getPokeballByServerId(uint16_t id);
		int32_t getPokeballId(const std::string& name) const;
		void clear();
		bool reload();

	private:
		std::map<uint16_t, Pokeball> pokeballsMap;
};

#endif
