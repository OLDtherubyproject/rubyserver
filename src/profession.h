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

#ifndef FS_PROFESSION_H_ADCAA356C0DB44CEBA994A0D678EC92D
#define FS_PROFESSION_H_ADCAA356C0DB44CEBA994A0D678EC92D

#include "enums.h"
#include "item.h"

class Profession
{
	public:
		explicit Profession(uint16_t id) : id(id) {}

		const std::string& getProfName() const {
			return name;
		}
		const std::string& getProfDescription() const {
			return description;
		}
		uint64_t getReqSkillTries(uint8_t skill, uint16_t level);

		uint16_t getId() const {
			return id;
		}

		uint8_t getClientId() const {
			return clientId;
		}

		uint32_t getHPGain() const {
			return gainHP;
		}
		uint32_t getCapGain() const {
			return gainCap;
		}

		uint32_t getHealthGainTicks() const {
			return gainHealthTicks;
		}
		uint32_t getHealthGainAmount() const {
			return gainHealthAmount;
		}

		uint32_t getBaseSpeed() const {
			return baseSpeed;
		}

		uint32_t getFromProfession() const {
			return fromProfession;
		}
	private:
		friend class Professions;

		std::map<uint32_t, uint32_t> cacheSkill[SKILL_LAST + 1];

		std::string name = "Pokemon Trainer";
		std::string description;

		float skillMultipliers[SKILL_LAST + 1] = {1.5f, 2.0f, 2.0f, 2.0f, 2.0f, 1.5f, 1.1f};

		uint32_t gainHealthTicks = 6;
		uint32_t gainHealthAmount = 1;
		uint32_t gainCap = 500;
		uint32_t gainHP = 5;
		uint32_t fromProfession = PROFESSION_NONE;
		uint32_t baseSpeed = 220;
		uint16_t id;

		uint8_t clientId = 0;

		static uint32_t skillBase[SKILL_LAST + 1];
};

class Professions
{
	public:
		bool loadFromXml();

		Profession* getProfession(uint16_t id);
		int32_t getProfessionId(const std::string& name) const;
		uint16_t getPromotedProfession(uint16_t professionId) const;

	private:
		std::map<uint16_t, Profession> professionsMap;
};

#endif
