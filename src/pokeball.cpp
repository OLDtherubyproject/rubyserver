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

		if ((attr = pokeballNode.attribute("defaultMultiplier"))) {
			ball.rates.all = pugi::cast<uint16_t>(attr.value());
		} else {
			std::cout << "[Warning - Pokeballs::loadFromXml] Missing pokeball defaultMultiplier percentage" << std::endl;
			continue;
		}

		if ((attr = pokeballNode.attribute("level"))) {
			ball.level = pugi::cast<uint32_t>(attr.value());
		}

		for (auto childNode : pokeballNode.children()) {
			if (strcasecmp(childNode.name(), "rate") == 0) {
				pugi::xml_attribute attrRateType = childNode.attribute("type");
                pugi::xml_attribute attrRateMultiplier = childNode.attribute("multiplier");
				if (attrRateType && attrRateMultiplier) {
                    std::string rateType = attrRateType.as_string();
                    double rateMultiplier = pugi::cast<double>(attrRateMultiplier.value());
					if (strcasecmp(rateType.c_str(), "fire") == 0) {
                        ball.rates.fire = rateMultiplier;
					} else if (strcasecmp(rateType.c_str(), "fighting") == 0) {
                        ball.rates.fighting = rateMultiplier;
					} else if (strcasecmp(rateType.c_str(), "water") == 0) {
                        ball.rates.water = rateMultiplier;
					} else if (strcasecmp(rateType.c_str(), "flying") == 0) {
                        ball.rates.flying = rateMultiplier;
					} else if (strcasecmp(rateType.c_str(), "grass") == 0) {
                        ball.rates.grass = rateMultiplier;
					} else if (strcasecmp(rateType.c_str(), "electric") == 0) {
                        ball.rates.electric = rateMultiplier;
					} else if (strcasecmp(rateType.c_str(), "poison") == 0) {
                        ball.rates.poison = rateMultiplier;
					} else if (strcasecmp(rateType.c_str(), "ground") == 0) {
                        ball.rates.ground = rateMultiplier;
					} else if (strcasecmp(rateType.c_str(), "psychic") == 0) {
                        ball.rates.psychic = rateMultiplier;
					} else if (strcasecmp(rateType.c_str(), "rock") == 0) {
                        ball.rates.rock = rateMultiplier;
					} else if (strcasecmp(rateType.c_str(), "ice") == 0) {
                        ball.rates.ice = rateMultiplier;
					} else if (strcasecmp(rateType.c_str(), "bug") == 0) {
                        ball.rates.bug = rateMultiplier;
					} else if (strcasecmp(rateType.c_str(), "dragon") == 0) {
                        ball.rates.dragon = rateMultiplier;
					} else if (strcasecmp(rateType.c_str(), "ghost") == 0) {
                        ball.rates.ghost = rateMultiplier;
					} else if (strcasecmp(rateType.c_str(), "dark") == 0) {
                        ball.rates.dark = rateMultiplier;
					} else if (strcasecmp(rateType.c_str(), "steel") == 0) {
                        ball.rates.steel = rateMultiplier;
					} else if (strcasecmp(rateType.c_str(), "fairy") == 0) {
                        ball.rates.fairy = rateMultiplier;
					} else if (strcasecmp(rateType.c_str(), "normal") == 0) {
                        ball.rates.normal = rateMultiplier;
					} else {
						std::cout << "[Notice - Pokeballs::loadFromXml] No valid rate type: " << rateType << " for pokeball: " << ball.id << std::endl;
					}
				} else {
					std::cout << "[Notice - Pokeballs::loadFromXml] Missing rate type or multiplier for pokeball: " << ball.id << std::endl;
				}
			}
		}
	}
	return true;
}

Pokeball* Pokeballs::getPokeball(uint16_t id)
{
	auto it = pokeballsMap.find(id);
	if (it == pokeballsMap.end()) {
		std::cout << "[Warning - Pokeballs::getPokeball] Pokeball " << id << " not found." << std::endl;
		return nullptr;
	}
	return &it->second;
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