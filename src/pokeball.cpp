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

#include "pokeball.h"

#include "pugicast.h"
#include "tools.h"

bool Pokeballs::loadFromXml()
{
	pugi::xml_document doc;
	pugi::xml_parse_result result = doc.load_file("data/XML/pokeballs.xml");
	if (!result) {
		printXMLError("Error - Pokeballs::loadFromXml", "data/XML/pokeballs.xml", result);
		return false;
	}

	for (auto pokeballNode : doc.child("pokeballs").children()) {
		pugi::xml_attribute attr;
		if (!(attr = pokeballNode.attribute("id"))) {
			std::cout << "[Warning - Pokeballs::loadFromXml] Missing pokeball id" << std::endl;
			continue;
		}

		uint16_t id = pugi::cast<uint16_t>(attr.value());

		auto res = pokeballsMap.emplace(std::piecewise_construct,
				                        std::forward_as_tuple(id), 
                                        std::forward_as_tuple(id));
		Pokeball& ball = res.first->second;

		if ((attr = pokeballNode.attribute("serverid"))) {
			ball.serverid = pugi::cast<uint16_t>(attr.value());
		} else {
			std::cout << "[Warning - Pokeballs::loadFromXml] Missing pokeball serverid" << std::endl;
			continue;
		}

		if ((attr = pokeballNode.attribute("name"))) {
			ball.name = attr.as_string();
		} else {
			std::cout << "[Warning - Pokeballs::loadFromXml] Missing pokeball name" << std::endl;
			continue;
		}

		if ((attr = pokeballNode.attribute("charged"))) {
			ball.charged = pugi::cast<uint16_t>(attr.value());
		} else {
			std::cout << "[Warning - Pokeballs::loadFromXml] Missing pokeball charged id" << std::endl;
			continue;
		}

		if ((attr = pokeballNode.attribute("discharged"))) {
			ball.discharged = pugi::cast<uint16_t>(attr.value());
		} else {
			std::cout << "[Warning - Pokeballs::loadFromXml] Missing discharged id" << std::endl;
			continue;
		}

		if ((attr = pokeballNode.attribute("goback"))) {
			ball.goback = pugi::cast<uint16_t>(attr.value());
		} else {
			std::cout << "[Warning - Pokeballs::loadFromXml] Missing pokeball goback effect id" << std::endl;
			continue;
		}

		if ((attr = pokeballNode.attribute("catchSuccess"))) {
			ball.catchSuccess = pugi::cast<uint16_t>(attr.value());
		} else {
			std::cout << "[Warning - Pokeballs::loadFromXml] Missing pokeball catchSuccess effect id" << std::endl;
			continue;
		}

		if ((attr = pokeballNode.attribute("catchFail"))) {
			ball.catchFail = pugi::cast<uint16_t>(attr.value());
		} else {
			std::cout << "[Warning - Pokeballs::loadFromXml] Missing pokeball catchFail effect id" << std::endl;
			continue;
		}

		if ((attr = pokeballNode.attribute("shotEffect"))) {
			ball.shotEffect = pugi::cast<uint16_t>(attr.value());
		} else {
			std::cout << "[Warning - Pokeballs::loadFromXml] Missing shot effect id" << std::endl;
			continue;
		}

		if ((attr = pokeballNode.attribute("rate"))) {
			ball.rate = pugi::cast<double>(attr.value());
		} else {
			std::cout << "[Warning - Pokeballs::loadFromXml] Missing pokeball default rate value" << std::endl;
			continue;
		}

		if ((attr = pokeballNode.attribute("level"))) {
			ball.level = pugi::cast<uint32_t>(attr.value());
		}
	}
	return true;
}

Pokeball* Pokeballs::getPokeball(uint16_t id)
{
	auto it = pokeballsMap.find(id);
	if (it == pokeballsMap.end()) {
		return nullptr;
	}
	return &it->second;
}

Pokeball* Pokeballs::getPokeballByServerId(uint16_t id)
{
	for (auto& it : pokeballsMap) {
		if (it.second.serverid == id) {
			return &it.second;
		}
	}
	return nullptr;
}

int32_t Pokeballs::getPokeballId(const std::string& name) const
{
	for (const auto& it : pokeballsMap) {
		if (strcasecmp(it.second.name.c_str(), name.c_str()) == 0) {
			return it.first;
		}
	}
	return -1;
}

void Pokeballs::clear()
{
	pokeballsMap.clear();
}

bool Pokeballs::reload()
{
	clear();

	if (!loadFromXml()) {
		return false;
	}

	return true;
}