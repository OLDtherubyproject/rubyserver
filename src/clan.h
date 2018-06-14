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

#ifndef FS_CLAN_H_W783RBCTQ73RTCB7URYF7W4CW47TRFGA
#define FS_CLAN_H_W783RBCTQ73RTCB7URYF7W4CW47TRFGA

#include "enums.h"
#include "item.h"

class Clan
{
	public:
		explicit Clan(uint16_t id) : id(id) {}

		const std::string& getClanName() const {
			return name;
		}
		const std::string& getClanDescription() const {
			return description;
		}

		uint16_t getId() const {
			return id;
		}

		uint8_t getClientId() const {
			return clientId;
		}

		uint32_t getFromClan() const {
			return fromClan;
		}

		float getTypeMultiplier(PokemonType_t type) const;
	private:
		friend class Clans;

		std::string name = "Pokemon Trainer";
		std::string description;

		float typeMultipliers[TYPE_LAST + 1] = {1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f};

		uint32_t fromClan = CLAN_NONE;
		uint16_t id;

		uint8_t clientId = 0;
};

class Clans
{
	public:
		bool loadFromXml();

		Clan* getClan(uint16_t id);
		int32_t getClanId(const std::string& name) const;
		uint16_t getPromotedClan(uint16_t clanId) const;

	private:
		std::map<uint16_t, Clan> clansMap;
};

#endif
