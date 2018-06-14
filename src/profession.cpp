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

#include "profession.h"

#include "pugicast.h"
#include "tools.h"

bool Professions::loadFromXml()
{
	pugi::xml_document doc;
	pugi::xml_parse_result result = doc.load_file("data/XML/professions.xml");
	if (!result) {
		printXMLError("Error - Professions::loadFromXml", "data/XML/professions.xml", result);
		return false;
	}

	for (auto professionNode : doc.child("professions").children()) {
		pugi::xml_attribute attr;
		if (!(attr = professionNode.attribute("id"))) {
			std::cout << "[Warning - Professions::loadFromXml] Missing profession id" << std::endl;
			continue;
		}

		uint16_t id = pugi::cast<uint16_t>(attr.value());

		auto res = professionsMap.emplace(std::piecewise_construct,
				std::forward_as_tuple(id), std::forward_as_tuple(id));
		Profession& prof = res.first->second;

		if ((attr = professionNode.attribute("name"))) {
			prof.name = attr.as_string();
		}

		if ((attr = professionNode.attribute("clientid"))) {
			prof.clientId = pugi::cast<uint16_t>(attr.value());
		}

		if ((attr = professionNode.attribute("description"))) {
			prof.description = attr.as_string();
		}

		if ((attr = professionNode.attribute("gaincap"))) {
			prof.gainCap = pugi::cast<uint32_t>(attr.value()) * 100;
		}

		if ((attr = professionNode.attribute("gainhp"))) {
			prof.gainHP = pugi::cast<uint32_t>(attr.value());
		}

		if ((attr = professionNode.attribute("gainhpticks"))) {
			prof.gainHealthTicks = pugi::cast<uint32_t>(attr.value());
		}

		if ((attr = professionNode.attribute("gainhpamount"))) {
			prof.gainHealthAmount = pugi::cast<uint32_t>(attr.value());
		}

		if ((attr = professionNode.attribute("basespeed"))) {
			prof.baseSpeed = pugi::cast<uint32_t>(attr.value());
		}

		if ((attr = professionNode.attribute("fromprof"))) {
			prof.fromProfession = pugi::cast<uint32_t>(attr.value());
		}

		for (auto childNode : professionNode.children()) {
			if (strcasecmp(childNode.name(), "skill") == 0) {
				pugi::xml_attribute skillIdAttribute = childNode.attribute("id");
				if (skillIdAttribute) {
					uint16_t skill_id = pugi::cast<uint16_t>(skillIdAttribute.value());
					if (skill_id <= SKILL_LAST) {
						prof.skillMultipliers[skill_id] = pugi::cast<float>(childNode.attribute("multiplier").value());
					} else {
						std::cout << "[Notice - Professions::loadFromXml] No valid skill id: " << skill_id << " for profession: " << prof.id << std::endl;
					}
				} else {
					std::cout << "[Notice - Professions::loadFromXml] Missing skill id for profession: " << prof.id << std::endl;
				}
			}
		}
	}
	return true;
}

Profession* Professions::getProfession(uint16_t id)
{
	auto it = professionsMap.find(id);
	if (it == professionsMap.end()) {
		std::cout << "[Warning - Professions::getProfession] Profession " << id << " not found." << std::endl;
		return nullptr;
	}
	return &it->second;
}

int32_t Professions::getProfessionId(const std::string& name) const
{
	for (const auto& it : professionsMap) {
		if (strcasecmp(it.second.name.c_str(), name.c_str()) == 0) {
			return it.first;
		}
	}
	return -1;
}

uint16_t Professions::getPromotedProfession(uint16_t professionId) const
{
	for (const auto& it : professionsMap) {
		if (it.second.fromProfession == professionId && it.first != professionId) {
			return it.first;
		}
	}
	return PROFESSION_NONE;
}

uint32_t Profession::skillBase[SKILL_LAST + 1] = {50, 50, 50, 50, 30, 100, 20};

uint64_t Profession::getReqSkillTries(uint8_t skill, uint16_t level)
{
	if (skill > SKILL_LAST) {
		return 0;
	}

	auto it = cacheSkill[skill].find(level);
	if (it != cacheSkill[skill].end()) {
		return it->second;
	}

	uint64_t tries = static_cast<uint64_t>(skillBase[skill] * std::pow(static_cast<double>(skillMultipliers[skill]), level - 11));
	cacheSkill[skill][level] = tries;
	return tries;
}
