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

#include "groups.h"

#include "pugicast.h"
#include "tools.h"

bool Groups::load()
{
	pugi::xml_document doc;
	pugi::xml_parse_result result = doc.load_file("data/XML/groups.xml");
	if (!result) {
		printXMLError("Error - Groups::load", "data/XML/groups.xml", result);
		return false;
	}

	for (auto groupNode : doc.child("groups").children()) {
		Group group;
		group.id = pugi::cast<uint32_t>(groupNode.attribute("id").value());
		group.name = groupNode.attribute("name").as_string();
		group.flags = pugi::cast<uint64_t>(groupNode.attribute("flags").value());
		group.access = groupNode.attribute("access").as_bool();
		group.maxDepotItems = pugi::cast<uint32_t>(groupNode.attribute("maxdepotitems").value());
		group.maxVipEntries = pugi::cast<uint32_t>(groupNode.attribute("maxvipentries").value());
		
		pugi::xml_attribute attr;
		if ((attr = groupNode.attribute("nameColor"))) {
			std::string tmpStrValue = asLowerCaseString(attr.as_string());
			uint16_t tmpInt = pugi::cast<uint16_t>(attr.value());
			if (tmpStrValue == "red" || tmpInt == 1) {
				group.nameColor = NAMECOLOR_RED;
			} else if (tmpStrValue == "orange" || tmpInt == 2) {
				group.nameColor = NAMECOLOR_ORANGE;
			} else if (tmpStrValue == "yellow" || tmpInt == 3) {
				group.nameColor = NAMECOLOR_YELLOW;
			} else if (tmpStrValue == "blue" || tmpInt == 4) {
				group.nameColor = NAMECOLOR_BLUE;
			} else if (tmpStrValue == "purple" || tmpInt == 5) {
				group.nameColor = NAMECOLOR_PURPLE;
			} else if (tmpStrValue == "white" || tmpInt == 6) {
				group.nameColor = NAMECOLOR_WHITE;
			} else if (tmpStrValue == "black" || tmpInt == 7) {
				group.nameColor = NAMECOLOR_BLACK;
			} else {
				std::cout << "[Warning - Groups::load] Unknown name color " << attr.as_string() << " for the group " << group.name << std::endl;
			}
		}

		groups.push_back(group);
	}
	return true;
}

Group* Groups::getGroup(uint16_t id)
{
	for (Group& group : groups) {
		if (group.id == id) {
			return &group;
		}
	}
	return nullptr;
}
