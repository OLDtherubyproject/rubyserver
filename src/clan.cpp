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

#include "clan.h"

#include "pugicast.h"
#include "tools.h"

bool Clans::loadFromXml()
{
	pugi::xml_document doc;
	pugi::xml_parse_result result = doc.load_file("data/XML/clans.xml");
	if (!result) {
		printXMLError("Error - Clans::loadFromXml", "data/XML/clans.xml", result);
		return false;
	}

	for (auto clanNode : doc.child("clans").children()) {
		pugi::xml_attribute attr;
		if (!(attr = clanNode.attribute("id"))) {
			std::cout << "[Warning - Clanss::loadFromXml] Missing clan id" << std::endl;
			continue;
		}

		uint16_t id = pugi::cast<uint16_t>(attr.value());

		auto res = clansMap.emplace(std::piecewise_construct,
				std::forward_as_tuple(id), std::forward_as_tuple(id));
		Clan& clan = res.first->second;

		if ((attr = clanNode.attribute("name"))) {
			clan.name = attr.as_string();
		}

		if ((attr = clanNode.attribute("clientid"))) {
			clan.clientId = pugi::cast<uint16_t>(attr.value());
		}

		if ((attr = clanNode.attribute("description"))) {
			clan.description = attr.as_string();
		}

		if ((attr = clanNode.attribute("fromclan"))) {
			clan.fromClan = pugi::cast<uint32_t>(attr.value());
		}

		for (auto childNode : clanNode.children()) {
			if (strcasecmp(childNode.name(), "type") == 0) {
				pugi::xml_attribute typeIdAttribute = childNode.attribute("id");
				if (typeIdAttribute) {
					uint16_t type_id = pugi::cast<uint16_t>(typeIdAttribute.value());
					if (type_id <= TYPE_LAST) {
						clan.typeMultipliers[type_id] = pugi::cast<float>(childNode.attribute("multiplier").value());
					} else {
						std::cout << "[Notice - Clans::loadFromXml] No valid type id: " << type_id << " for clan: " << clan.id << std::endl;
					}
				} else {
					std::cout << "[Notice - Clans::loadFromXml] Missing type id for clan: " << clan.id << std::endl;
				}
			}
		}
	}
	return true;
}

Clan* Clans::getClan(uint16_t id)
{
	auto it = clansMap.find(id);
	if (it == clansMap.end()) {
		std::cout << "[Warning - Clans::getClan] Clan " << id << " not found." << std::endl;
		return nullptr;
	}
	return &it->second;
}

int32_t Clans::getClanId(const std::string& name) const
{
	for (const auto& it : clansMap) {
		if (strcasecmp(it.second.name.c_str(), name.c_str()) == 0) {
			return it.first;
		}
	}
	return -1;
}

uint16_t Clans::getPromotedClan(uint16_t clanId) const
{
	for (const auto& it : clansMap) {
		if (it.second.fromClan == clanId && it.first != clanId) {
			return it.first;
		}
	}
	return CLAN_NONE;
}


float Clan::getTypeMultiplier(PokemonType_t type) const
{
	if (type > TYPE_LAST) {
		return 1.0f;
	}

	return typeMultipliers[type];
}