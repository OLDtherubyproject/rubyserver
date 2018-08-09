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

#include "pokemons.h"
#include "pokemon.h"
#include "moves.h"
#include "combat.h"
#include "configmanager.h"
#include "game.h"

#include "pugicast.h"

extern Game g_game;
extern Moves* g_moves;
extern Pokemons g_pokemons;
extern ConfigManager g_config;

uint32_t Pokemons::getLootRandom()
{
	return uniform_random(0, MAX_LOOTCHANCE) / g_config.getNumber(ConfigManager::RATE_LOOT);
}

void PokemonType::createLoot(Container* corpse)
{
	if (g_config.getNumber(ConfigManager::RATE_LOOT) == 0) {
		corpse->startDecaying();
		return;
	}

	Player* owner = g_game.getPlayerByID(corpse->getCorpseOwner());
	if (!owner || owner->getStaminaMinutes() > 840) {
		for (auto it = info.lootItems.rbegin(), end = info.lootItems.rend(); it != end; ++it) {
			auto itemList = createLootItem(*it);
			if (itemList.empty()) {
				continue;
			}

			for (Item* item : itemList) {
				//check containers
				if (Container* container = item->getContainer()) {
					if (!createLootContainer(container, *it)) {
						delete container;
						continue;
					}
				}

				if (g_game.internalAddItem(corpse, item) != RETURNVALUE_NOERROR) {
					corpse->internalAddThing(item);
				}
			}
		}

		if (owner) {
			std::ostringstream ss;
			ss << "Loot of " << nameDescription << ": " << corpse->getContentDescription();

			if (owner->getParty()) {
				owner->getParty()->broadcastPartyLoot(ss.str());
			} else {
				owner->sendTextMessage(MESSAGE_LOOT, ss.str());
			}
		}
	} else {
		std::ostringstream ss;
		ss << "Loot of " << nameDescription << ": nothing (due to low stamina)";

		if (owner->getParty()) {
			owner->getParty()->broadcastPartyLoot(ss.str());
		} else {
			owner->sendTextMessage(MESSAGE_LOOT, ss.str());
		}
	}

	corpse->startDecaying();
}

std::vector<Item*> PokemonType::createLootItem(const LootBlock& lootBlock)
{
	int32_t itemCount = 0;
	uint16_t itemMaxCount = 1;

	uint32_t randvalue = Pokemons::getLootRandom();
	if (randvalue < lootBlock.chance) {
		if (Item::items[lootBlock.id].stackable) {
			itemCount = randvalue % lootBlock.countmax + 1;
			itemMaxCount = Item::items[lootBlock.id].getItemMaxCount();
		} else {
			itemCount = 1;
			itemMaxCount = 1;
		}
	}

	std::vector<Item*> itemList;
	while (itemCount > 0) {
		uint16_t n = static_cast<uint16_t>(std::min<int32_t>(itemCount, itemMaxCount));
		Item* tmpItem = Item::CreateItem(lootBlock.id, n);
		if (!tmpItem) {
			break;
		}

		itemCount -= n;

		if (lootBlock.subType != -1) {
			tmpItem->setSubType(lootBlock.subType);
		}

		if (lootBlock.actionId != -1) {
			tmpItem->setActionId(lootBlock.actionId);
		}

		if (!lootBlock.text.empty()) {
			tmpItem->setText(lootBlock.text);
		}

		itemList.push_back(tmpItem);
	}
	return itemList;
}

bool PokemonType::createLootContainer(Container* parent, const LootBlock& lootblock)
{
	auto it = lootblock.childLoot.begin(), end = lootblock.childLoot.end();
	if (it == end) {
		return true;
	}

	for (; it != end && parent->size() < parent->capacity(); ++it) {
		auto itemList = createLootItem(*it);
		for (Item* tmpItem : itemList) {
			if (Container* container = tmpItem->getContainer()) {
				if (!createLootContainer(container, *it)) {
					delete container;
				} else {
					parent->internalAddThing(container);
				}
			} else {
				parent->internalAddThing(tmpItem);
			}
		}
	}
	return !parent->empty();
}

bool Pokemons::loadFromXml(bool reloading /*= false*/)
{
	unloadedPokemons = {};
	pugi::xml_document doc;
	pugi::xml_parse_result result = doc.load_file("data/pokemon/pokemon.xml");
	if (!result) {
		printXMLError("Error - Pokemons::loadFromXml", "data/pokemon/pokemon.xml", result);
		return false;
	}

	loaded = true;

	for (auto pokemonNode : doc.child("pokemon").children()) {
		std::string name = asLowerCaseString(pokemonNode.attribute("name").as_string());
		std::string region = std::string(pokemonNode.attribute("region").as_string());
		std::string file = "data/pokemon/" + region + "/" + std::string(pokemonNode.attribute("file").as_string());
		if (reloading && pokemons.find(name) != pokemons.end()) {
			loadPokemon(file, name, true);
		} else {
			unloadedPokemons.emplace(name, file);
		}
	}
	return true;
}

bool Pokemons::reload()
{
	loaded = false;

	scriptInterface.reset();

	return loadFromXml(true);
}

ConditionDamage* Pokemons::getDamageCondition(ConditionType_t conditionType,
        int32_t maxDamage, int32_t minDamage, int32_t startDamage, uint32_t tickInterval)
{
	ConditionDamage* condition = static_cast<ConditionDamage*>(Condition::createCondition(CONDITIONID_COMBAT, conditionType, 0, 0));
	condition->setParam(CONDITION_PARAM_TICKINTERVAL, tickInterval);
	condition->setParam(CONDITION_PARAM_MINVALUE, minDamage);
	condition->setParam(CONDITION_PARAM_MAXVALUE, maxDamage);
	condition->setParam(CONDITION_PARAM_STARTVALUE, startDamage);
	condition->setParam(CONDITION_PARAM_DELAYED, 1);
	return condition;
}

PokemonType* Pokemons::loadPokemon(const std::string& file, const std::string& pokemonName, bool reloading /*= false*/)
{
	PokemonType* mType = nullptr;

	pugi::xml_document doc;
	pugi::xml_parse_result result = doc.load_file(file.c_str());
	if (!result) {
		printXMLError("Error - Pokemons::loadPokemon", file, result);
		return nullptr;
	}

	pugi::xml_node pokemonNode = doc.child("pokemon");
	if (!pokemonNode) {
		std::cout << "[Error - Pokemons::loadPokemon] Missing pokemon node in: " << file << std::endl;
		return nullptr;
	}

	pugi::xml_attribute attr;
	if (!(attr = pokemonNode.attribute("name"))) {
		std::cout << "[Error - Pokemons::loadPokemon] Missing name in: " << file << std::endl;
		return nullptr;
	}

	if (reloading) {
		auto it = pokemons.find(asLowerCaseString(pokemonName));
		if (it != pokemons.end()) {
			mType = &it->second;
			mType->info = {};
		}
	}

	if (!mType) {
		mType = &pokemons[asLowerCaseString(pokemonName)];
	}

	mType->name = attr.as_string();
	mType->typeName = pokemonName;

	if ((attr = pokemonNode.attribute("nameColor"))) {
		std::string tmpStrValue = asLowerCaseString(attr.as_string());
		uint16_t tmpInt = pugi::cast<uint16_t>(attr.value());
		if (tmpStrValue == "red" || tmpInt == 1) {
			mType->nameColor = NAMECOLOR_RED;
		} else if (tmpStrValue == "orange" || tmpInt == 2) {
			mType->nameColor = NAMECOLOR_ORANGE;
		} else if (tmpStrValue == "yellow" || tmpInt == 3) {
			mType->nameColor = NAMECOLOR_YELLOW;
		} else if (tmpStrValue == "blue" || tmpInt == 4) {
			mType->nameColor = NAMECOLOR_BLUE;
		} else if (tmpStrValue == "purple" || tmpInt == 5) {
			mType->nameColor = NAMECOLOR_PURPLE;
		} else if (tmpStrValue == "white" || tmpInt == 6) {
			mType->nameColor = NAMECOLOR_WHITE;
		} else if (tmpStrValue == "black" || tmpInt == 7) {
			mType->nameColor = NAMECOLOR_BLACK;
		} else {
			std::cout << "[Warning - Pokemons::loadPokemon] Unknown name color " << attr.as_string() << ". " << file << std::endl;
		}
	}

	if ((attr = pokemonNode.attribute("number"))) {
		mType->number = pugi::cast<uint16_t>(attr.value());
	} else {
		std::cout << "[Warning - Pokemons::loadPokemon] Number invalid. " << file << std::endl;
	}

	if ((attr = pokemonNode.attribute("nameDescription"))) {
		mType->nameDescription = attr.as_string();
	} else {
		mType->nameDescription = "a " + asLowerCaseString(mType->name);
	}

	if ((attr = pokemonNode.attribute("blood"))) {
		std::string tmpStrValue = asLowerCaseString(attr.as_string());
		uint16_t tmpInt = pugi::cast<uint16_t>(attr.value());
		if (tmpStrValue == "red" || tmpInt == 1) {
			mType->info.blood = BLOOD_RED;
		} else if (tmpStrValue == "green" || tmpInt == 2) {
			mType->info.blood = BLOOD_GREEN;
		} else if (tmpStrValue == "gray" || tmpInt == 3) {
			mType->info.blood = BLOOD_GRAY;
		} else if (tmpStrValue == "blue" || tmpInt == 4) {
			mType->info.blood = BLOOD_BLUE;
		} else if (tmpStrValue == "purple" || tmpInt == 5) {
			mType->info.blood = BLOOD_PURPLE;
		} else {
			std::cout << "[Warning - Pokemons::loadPokemon] Unknown blood type " << attr.as_string() << ". " << file << std::endl;
		}
	}

	if ((attr = pokemonNode.attribute("experience"))) {
		mType->info.experience = pugi::cast<uint64_t>(attr.value());
	}

	if ((attr = pokemonNode.attribute("catchRate"))) {
		mType->info.catchRate = pugi::cast<double>(attr.value());
	}

	if ((attr = pokemonNode.attribute("price"))) {
		mType->info.price = pugi::cast<int32_t>(attr.value());
	}

	if ((attr = pokemonNode.attribute("level"))) {
		mType->info.level = pugi::cast<int32_t>(attr.value());
	}

	if ((attr = pokemonNode.attribute("gender"))) {
		mType->info.gender = getGenderType(asLowerCaseString(attr.as_string()));
	}

	if ((attr = pokemonNode.attribute("script"))) {
		if (!scriptInterface) {
			scriptInterface.reset(new LuaScriptInterface("Pokemon Interface"));
			scriptInterface->initState();
		}

		std::string script = attr.as_string();
		if (scriptInterface->loadFile("data/pokemon/scripts/" + script) == 0) {
			mType->info.scriptInterface = scriptInterface.get();
			mType->info.creatureAppearEvent = scriptInterface->getEvent("onCreatureAppear");
			mType->info.creatureDisappearEvent = scriptInterface->getEvent("onCreatureDisappear");
			mType->info.creatureMoveEvent = scriptInterface->getEvent("onCreatureMove");
			mType->info.creatureSayEvent = scriptInterface->getEvent("onCreatureSay");
			mType->info.thinkEvent = scriptInterface->getEvent("onThink");
		} else {
			std::cout << "[Warning - Pokemons::loadPokemon] Can not load script: " << script << std::endl;
			std::cout << scriptInterface->getLastLuaError() << std::endl;
		}
	}

	pugi::xml_node node;
	if ((node = pokemonNode.child("type"))) {
		if ((attr = node.attribute("first"))) {
			mType->info.firstType = getPokemonElementType(asLowerCaseString(attr.as_string()));
		} else {
			std::cout << "[Error - Pokemons::loadPokemon] Missing first type. " << file << std::endl;
		}

		if ((attr = node.attribute("second"))) {
			mType->info.secondType = getPokemonElementType(asLowerCaseString(attr.as_string()));
		}
	} else {
		std::cout << "[Error - Pokemons::loadPokemon] Missing type(s). " << file << std::endl;
	}

	if ((node = pokemonNode.child("basestats"))) {
		pugi::xml_node auxNode;
		if ((auxNode = node.child("hp"))) {
			if ((attr = auxNode.attribute("value"))) {
				mType->info.baseStats.hp = pugi::cast<uint32_t>(attr.value());
			} else {
				std::cout << "[Warning - Pokemons::loadPokemon] Missing hp value basestats. " << file << std::endl;
			}
		} else {
			std::cout << "[Warning - Pokemons::loadPokemon] Missing hp basestats. " << file << std::endl;
		}

		if ((auxNode = node.child("attack"))) {
			if ((attr = auxNode.attribute("value"))) {
				mType->info.baseStats.attack = pugi::cast<uint32_t>(attr.value());
			} else {
				std::cout << "[Warning - Pokemons::loadPokemon] Missing attack value basestats. " << file << std::endl;
			}
		} else {
			std::cout << "[Warning - Pokemons::loadPokemon] Missing attack basestats. " << file << std::endl;
		}

		if ((auxNode = node.child("defense"))) {
			if ((attr = auxNode.attribute("value"))) {
				mType->info.baseStats.defense = pugi::cast<uint32_t>(attr.value());
			} else {
				std::cout << "[Warning - Pokemons::loadPokemon] Missing defense value basestats. " << file << std::endl;
			}
		} else {
			std::cout << "[Warning - Pokemons::loadPokemon] Missing defense basestats. " << file << std::endl;
		}

		if ((auxNode = node.child("specialAttack"))) {
			if ((attr = auxNode.attribute("value"))) {
				mType->info.baseStats.special_attack = pugi::cast<uint32_t>(attr.value());
			} else {
				std::cout << "[Warning - Pokemons::loadPokemon] Missing specialAttack value basestats. " << file << std::endl;
			}
		} else {
			std::cout << "[Warning - Pokemons::loadPokemon] Missing specialAttack basestats. " << file << std::endl;
		}

		if ((auxNode = node.child("specialDefense"))) {
			if ((attr = auxNode.attribute("value"))) {
				mType->info.baseStats.special_defense = pugi::cast<uint32_t>(attr.value());
			} else {
				std::cout << "[Warning - Pokemons::loadPokemon] Missing specialDefense value basestats. " << file << std::endl;
			}
		} else {
			std::cout << "[Warning - Pokemons::loadPokemon] Missing specialDefense basestats. " << file << std::endl;
		}

		if ((auxNode = node.child("speed"))) {
			if ((attr = auxNode.attribute("value"))) {
				mType->info.baseStats.speed = pugi::cast<uint32_t>(attr.value());
			} else {
				std::cout << "[Warning - Pokemons::loadPokemon] Missing speed value basestats. " << file << std::endl;
			}
		} else {
			std::cout << "[Warning - Pokemons::loadPokemon] Missing speed basestats. " << file << std::endl;
		}
	}

	if ((node = pokemonNode.child("flags"))) {
		for (auto flagNode : node.children()) {
			attr = flagNode.first_attribute();
			const char* attrName = attr.name();
			if (strcasecmp(attrName, "catchable") == 0) {
				mType->info.isCatchable = attr.as_bool();
			} else if (strcasecmp(attrName, "attackable") == 0) {
				mType->info.isAttackable = attr.as_bool();
			} else if (strcasecmp(attrName, "hostile") == 0) {
				mType->info.isHostile = attr.as_bool();
			} else if (strcasecmp(attrName, "illusionable") == 0) {
				mType->info.isIllusionable = attr.as_bool();
			} else if (strcasecmp(attrName, "convinceable") == 0) {
				mType->info.isConvinceable = attr.as_bool();
			} else if (strcasecmp(attrName, "pushable") == 0) {
				mType->info.pushable = attr.as_bool();
			} else if (strcasecmp(attrName, "canpushitems") == 0) {
				mType->info.canPushItems = attr.as_bool();
			} else if (strcasecmp(attrName, "canpushcreatures") == 0) {
				mType->info.canPushCreatures = attr.as_bool();
			} else if (strcasecmp(attrName, "staticattack") == 0) {
				uint32_t staticAttack = pugi::cast<uint32_t>(attr.value());
				if (staticAttack > 100) {
					std::cout << "[Warning - Pokemons::loadPokemon] staticattack greater than 100. " << file << std::endl;
					staticAttack = 100;
				}

				mType->info.staticAttackChance = staticAttack;
			} else if (strcasecmp(attrName, "lightlevel") == 0) {
				mType->info.light.level = pugi::cast<uint16_t>(attr.value());
			} else if (strcasecmp(attrName, "lightcolor") == 0) {
				mType->info.light.color = pugi::cast<uint16_t>(attr.value());
			} else if (strcasecmp(attrName, "targetdistance") == 0) {
				mType->info.targetDistance = std::max<int32_t>(1, pugi::cast<int32_t>(attr.value()));
			} else if (strcasecmp(attrName, "runonhealth") == 0) {
				mType->info.runAwayHealth = pugi::cast<int32_t>(attr.value());
			} else if (strcasecmp(attrName, "hidehealth") == 0) {
				mType->info.hiddenHealth = attr.as_bool();
			} else if (strcasecmp(attrName, "canwalkonenergy") == 0) {
				mType->info.canWalkOnEnergy = attr.as_bool();
			} else if (strcasecmp(attrName, "canwalkonfire") == 0) {
				mType->info.canWalkOnFire = attr.as_bool();
			} else if (strcasecmp(attrName, "canwalkonpoison") == 0) {
				mType->info.canWalkOnPoison = attr.as_bool();
			} else if (strcasecmp(attrName, "xpondex") == 0) {
				mType->info.xpOnDex = std::max<int32_t>(0, pugi::cast<int32_t>(attr.value()));
			} else {
				std::cout << "[Warning - Pokemons::loadPokemon] Unknown flag attribute: " << attrName << ". " << file << std::endl;
			}
		}

		//if a pokemon can push creatures,
		// it should not be pushable
		if (mType->info.canPushCreatures) {
			mType->info.pushable = false;
		}
	}

	if ((node = pokemonNode.child("portrait"))) {
		if ((attr = node.attribute("id"))) {
			mType->info.portrait = pugi::cast<uint32_t>(attr.value());
		} else {
			std::cout << "[Warning - Pokemons::loadPokemon] Missing portrait id. " << file << std::endl;
		}
	} else{
		std::cout << "[Warning - Pokemons::loadPokemon] Missing portrait. " << file << std::endl;
	}

	if ((node = pokemonNode.child("icon"))) {
		if ((attr = node.attribute("charged"))) {
			mType->info.iconCharged = pugi::cast<uint32_t>(attr.value());
		} else {
			std::cout << "[Warning - Pokemons::loadPokemon] Missing icon charged id. " << file << std::endl;
		}

		if ((attr = node.attribute("discharged"))) {
			mType->info.iconDischarged = pugi::cast<int32_t>(attr.value());
		} else {
			std::cout << "[Warning - Pokemons::loadPokemon] Missing icon discharged id. " << file << std::endl;
		}
	} else{
		std::cout << "[Warning - Pokemons::loadPokemon] Missing icon. " << file << std::endl;
	}

	if ((node = pokemonNode.child("targetchange"))) {
		if ((attr = node.attribute("speed")) || (attr = node.attribute("interval"))) {
			mType->info.changeTargetSpeed = pugi::cast<uint32_t>(attr.value());
		} else {
			std::cout << "[Warning - Pokemons::loadPokemon] Missing targetchange speed. " << file << std::endl;
		}

		if ((attr = node.attribute("chance"))) {
			mType->info.changeTargetChance = pugi::cast<int32_t>(attr.value());
		} else {
			std::cout << "[Warning - Pokemons::loadPokemon] Missing targetchange chance. " << file << std::endl;
		}
	}

	if ((node = pokemonNode.child("dittochance"))) {
		if ((attr = node.attribute("chance"))) {
			mType->info.dittoChance = pugi::cast<float>(attr.value());
		} else {
			std::cout << "[Warning - Pokemons::loadPokemon] Missing dittochance chance. " << file << std::endl;
		}
	}

	if ((node = pokemonNode.child("genders"))) {
		float sum = 0;
		for (auto genderNode : node.children()) {
			if ((attr = genderNode.attribute("name"))) {
				if ((strcasecmp(attr.value(), "male") == 0)) {
					if ((attr = genderNode.attribute("percentage"))) {
						float percentage = pugi::cast<float>(attr.value());;
						sum += percentage;
						mType->info.genders.male = percentage;
					} else {
						std::cout << "[Warning - Pokemons::loadPokemon] Gender percentage is missing in: " << attr.value() << ". " << file << std::endl;	
					}
				} else if (strcasecmp(attr.value(), "female") == 0) {
					if ((attr = genderNode.attribute("percentage"))) {
						float percentage = pugi::cast<float>(attr.value());;
						sum += percentage;
						mType->info.genders.female = percentage;
					} else {
						std::cout << "[Warning - Pokemons::loadPokemon] Gender percentage is missing in: " << attr.value() << ". " << file << std::endl;
					}
				} else{
					std::cout << "[Warning - Pokemons::loadPokemon] Unknown gender name: " << attr.value() << ". " << file << std::endl;
				}
			} else{
				std::cout << "[Warning - Pokemons::loadPokemon] Gender name is missing in " << file << std::endl;
			}			
		}

		if (sum > 100) {
			std::cout << "[Warning - Pokemons::loadPokemon] Gender total percentage is greater than 100 in " << file << std::endl;
		} else if (sum < 100) {
			std::cout << "[Warning - Pokemons::loadPokemon] Gender total percentage is less than 100 in " << file << std::endl;
		}
	}

	if ((node = pokemonNode.child("look"))) {
		if ((attr = node.attribute("type"))) {
			mType->info.outfit.lookType = pugi::cast<uint16_t>(attr.value());

			if ((attr = node.attribute("head"))) {
				mType->info.outfit.lookHead = pugi::cast<uint16_t>(attr.value());
			}

			if ((attr = node.attribute("body"))) {
				mType->info.outfit.lookBody = pugi::cast<uint16_t>(attr.value());
			}

			if ((attr = node.attribute("legs"))) {
				mType->info.outfit.lookLegs = pugi::cast<uint16_t>(attr.value());
			}

			if ((attr = node.attribute("feet"))) {
				mType->info.outfit.lookFeet = pugi::cast<uint16_t>(attr.value());
			}

			if ((attr = node.attribute("addons"))) {
				mType->info.outfit.lookAddons = pugi::cast<uint16_t>(attr.value());
			}
		} else if ((attr = node.attribute("typeex"))) {
			mType->info.outfit.lookTypeEx = pugi::cast<uint16_t>(attr.value());
		} else {
			std::cout << "[Warning - Pokemons::loadPokemon] Missing look type/typeex. " << file << std::endl;
		}

		if ((attr = node.attribute("mount"))) {
			mType->info.outfit.lookMount = pugi::cast<uint16_t>(attr.value());
		}

		if ((attr = node.attribute("corpse"))) {
			mType->info.lookcorpse = pugi::cast<uint16_t>(attr.value());
		}
	}

	if ((node = pokemonNode.child("spawn"))) {
		if ((attr = node.attribute("at"))) {
			std::string at = pugi::cast<std::string>(attr.value());

			if (at == "anytime") {
				mType->info.spawnAt = 0;
			}

			if (at == "night") {
				mType->info.spawnAt = 1;
			}

			if (at == "day") {
				mType->info.spawnAt = 2;
			}
		} else {
			std::cout << "[Warning - Pokemons::loadPokemon] Missing spawn at. " << file << std::endl;
		}
	}

	if ((node = pokemonNode.child("moves"))) {
		for (auto moveNode : node.children()) {
			Move* move;
			uint16_t chanceToLearn = 100;
			uint16_t chanceToCast = 100;

			if ((attr = moveNode.attribute("name"))) {
				move = g_moves->getMoveByName(attr.as_string());

				if (!move) {
					std::cout << "[Warning - Pokemons::loadPokemon] Move " << attr.as_string() << " not found at. " << file << std::endl;
					continue;
				}
			} else{
				std::cout << "[Warning - Pokemons::loadPokemon] Missing move name at. " << file << std::endl;
				continue;
			}

			if ((attr = moveNode.attribute("chanceToLearn"))) {
				chanceToLearn = pugi::cast<uint16_t>(attr.value());
			}

			if ((attr = moveNode.attribute("chanceToCast"))) {
				chanceToCast = pugi::cast<uint16_t>(attr.value());
			}

			mType->info.moves[move->getId()] = std::make_pair(chanceToLearn, chanceToCast);
		}
	}

	// default immunities
	if (mType->info.firstType == TYPE_NORMAL || mType->info.secondType == TYPE_NORMAL) {
		mType->info.damageImmunities |= COMBAT_GHOSTDAMAGE;
		mType->info.damageSuperEffective |= COMBAT_FIGHTINGDAMAGE;
	}
	if (mType->info.firstType == TYPE_GHOST || mType->info.secondType == TYPE_GHOST) {
		mType->info.damageImmunities |= COMBAT_NORMALDAMAGE;
		mType->info.damageImmunities |= COMBAT_FIGHTINGDAMAGE;
		mType->info.damageSuperEffective |= COMBAT_GHOSTDAMAGE;
		mType->info.damageSuperEffective |= COMBAT_DARKDAMAGE;
		mType->info.damageNotVeryEffective |= COMBAT_POISONDAMAGE;
		mType->info.damageNotVeryEffective |= COMBAT_BUGDAMAGE;
	}
	if (mType->info.firstType == TYPE_GROUND || mType->info.secondType == TYPE_GROUND) {
		mType->info.damageImmunities |= COMBAT_ELECTRICDAMAGE;
		mType->info.damageSuperEffective |= COMBAT_WATERDAMAGE;
		mType->info.damageSuperEffective |= COMBAT_GRASSDAMAGE;
		mType->info.damageSuperEffective |= COMBAT_ICEDAMAGE;
		mType->info.damageNotVeryEffective |= COMBAT_POISONDAMAGE;
		mType->info.damageNotVeryEffective |= COMBAT_ROCKDAMAGE;
	}
	if (mType->info.firstType == TYPE_DARK || mType->info.secondType == TYPE_DARK) {
		mType->info.damageImmunities |= COMBAT_PSYCHICDAMAGE;
		mType->info.damageSuperEffective |= COMBAT_FIGHTINGDAMAGE;
		mType->info.damageSuperEffective |= COMBAT_BUGDAMAGE;
		mType->info.damageSuperEffective |= COMBAT_FAIRYDAMAGE;
		mType->info.damageNotVeryEffective |= COMBAT_GHOSTDAMAGE;
		mType->info.damageNotVeryEffective |= COMBAT_DARKDAMAGE;
	}
	if (mType->info.firstType == TYPE_STEEL || mType->info.secondType == TYPE_STEEL) {
		mType->info.damageImmunities |= COMBAT_POISONDAMAGE;
		mType->info.damageSuperEffective |= COMBAT_FIGHTINGDAMAGE;
		mType->info.damageSuperEffective |= COMBAT_FIREDAMAGE;
		mType->info.damageSuperEffective |= COMBAT_GROUNDDAMAGE;
		mType->info.damageNotVeryEffective |= COMBAT_NORMALDAMAGE;
		mType->info.damageNotVeryEffective |= COMBAT_GRASSDAMAGE;
		mType->info.damageNotVeryEffective |= COMBAT_ICEDAMAGE;
		mType->info.damageNotVeryEffective |= COMBAT_FLYINGDAMAGE;
		mType->info.damageNotVeryEffective |= COMBAT_PSYCHICDAMAGE;
		mType->info.damageNotVeryEffective |= COMBAT_BUGDAMAGE;
		mType->info.damageNotVeryEffective |= COMBAT_ROCKDAMAGE;
		mType->info.damageNotVeryEffective |= COMBAT_DRAGONDAMAGE;
		mType->info.damageNotVeryEffective |= COMBAT_STEELDAMAGE;
		mType->info.damageNotVeryEffective |= COMBAT_FAIRYDAMAGE;
	}
	if (mType->info.firstType == TYPE_FAIRY || mType->info.secondType == TYPE_FAIRY) {
		mType->info.damageImmunities |= COMBAT_DRAGONDAMAGE;
		mType->info.damageSuperEffective |= COMBAT_POISONDAMAGE;
		mType->info.damageSuperEffective |= COMBAT_STEELDAMAGE;
		mType->info.damageNotVeryEffective |= COMBAT_FIGHTINGDAMAGE;
		mType->info.damageNotVeryEffective |= COMBAT_BUGDAMAGE;
		mType->info.damageNotVeryEffective |= COMBAT_DARKDAMAGE;
	}
	if (mType->info.firstType == TYPE_FLYING || mType->info.secondType == TYPE_FLYING) {
		mType->info.damageImmunities |= COMBAT_GROUNDDAMAGE;
		mType->info.damageSuperEffective |= COMBAT_ELECTRICDAMAGE;
		mType->info.damageSuperEffective |= COMBAT_ICEDAMAGE;
		mType->info.damageSuperEffective |= COMBAT_ROCKDAMAGE;
		mType->info.damageNotVeryEffective |= COMBAT_GRASSDAMAGE;
		mType->info.damageNotVeryEffective |= COMBAT_FIGHTINGDAMAGE;
		mType->info.damageNotVeryEffective |= COMBAT_BUGDAMAGE;
	}
	if (mType->info.firstType == TYPE_GRASS || mType->info.secondType == TYPE_GRASS) {
		mType->info.conditionImmunities |= CONDITION_SLEEP;
		mType->info.damageSuperEffective |= COMBAT_FIREDAMAGE;
		mType->info.damageSuperEffective |= COMBAT_ICEDAMAGE;
		mType->info.damageSuperEffective |= COMBAT_POISONDAMAGE;
		mType->info.damageSuperEffective |= COMBAT_FLYINGDAMAGE;
		mType->info.damageSuperEffective |= COMBAT_BUGDAMAGE;
		mType->info.damageNotVeryEffective |= COMBAT_GRASSDAMAGE;
		mType->info.damageNotVeryEffective |= COMBAT_WATERDAMAGE;
		mType->info.damageNotVeryEffective |= COMBAT_ELECTRICDAMAGE;
		mType->info.damageNotVeryEffective |= COMBAT_GROUNDDAMAGE;

	}
	if (mType->info.firstType == TYPE_POISON || mType->info.secondType == TYPE_POISON) {
		mType->info.conditionImmunities |= CONDITION_POISON;
		mType->info.damageSuperEffective |= COMBAT_GROUNDDAMAGE;
		mType->info.damageSuperEffective |= COMBAT_PSYCHICDAMAGE;
		mType->info.damageNotVeryEffective |= COMBAT_GRASSDAMAGE;
		mType->info.damageNotVeryEffective |= COMBAT_FIGHTINGDAMAGE;
		mType->info.damageNotVeryEffective |= COMBAT_POISONDAMAGE;
		mType->info.damageNotVeryEffective |= COMBAT_BUGDAMAGE;
		mType->info.damageNotVeryEffective |= COMBAT_FAIRYDAMAGE;
	}
	if (mType->info.firstType == TYPE_ELECTRIC || mType->info.secondType == TYPE_ELECTRIC) {
		mType->info.conditionImmunities |= CONDITION_PARALYZE;
		mType->info.damageSuperEffective |= COMBAT_GROUNDDAMAGE;
		mType->info.damageNotVeryEffective |= COMBAT_ELECTRICDAMAGE;
		mType->info.damageNotVeryEffective |= COMBAT_STEELDAMAGE;
		mType->info.damageNotVeryEffective |= COMBAT_FLYINGDAMAGE;
	}
	if (mType->info.firstType == TYPE_ICE || mType->info.secondType == TYPE_ICE) {
		mType->info.conditionImmunities |= CONDITION_FREEZING;
		mType->info.damageSuperEffective |= COMBAT_FIREDAMAGE;
		mType->info.damageSuperEffective |= COMBAT_FIGHTINGDAMAGE;
		mType->info.damageSuperEffective |= COMBAT_ROCKDAMAGE;
		mType->info.damageSuperEffective |= COMBAT_STEELDAMAGE;
		mType->info.damageNotVeryEffective |= COMBAT_ICEDAMAGE;
	}
	if (mType->info.firstType == TYPE_WATER || mType->info.secondType == TYPE_WATER) {
		mType->info.conditionImmunities |= CONDITION_DROWN;
		mType->info.damageSuperEffective |= COMBAT_ELECTRICDAMAGE;
		mType->info.damageSuperEffective |= COMBAT_GRASSDAMAGE;
		mType->info.damageNotVeryEffective |= COMBAT_STEELDAMAGE;
		mType->info.damageNotVeryEffective |= COMBAT_FIREDAMAGE;
		mType->info.damageNotVeryEffective |= COMBAT_WATERDAMAGE;
		mType->info.damageNotVeryEffective |= COMBAT_ICEDAMAGE;
	}
	if (mType->info.firstType == TYPE_FIGHTING || mType->info.secondType == TYPE_FIGHTING) {
		mType->info.damageSuperEffective |= COMBAT_FLYINGDAMAGE;
		mType->info.damageSuperEffective |= COMBAT_PSYCHICDAMAGE;
		mType->info.damageSuperEffective |= COMBAT_FAIRYDAMAGE;
		mType->info.damageNotVeryEffective |= COMBAT_BUGDAMAGE;
		mType->info.damageNotVeryEffective |= COMBAT_ROCKDAMAGE;
		mType->info.damageNotVeryEffective |= COMBAT_DARKDAMAGE;
	}
	if (mType->info.firstType == TYPE_PSYCHIC || mType->info.secondType == TYPE_PSYCHIC) {
		mType->info.damageSuperEffective |= COMBAT_BUGDAMAGE;
		mType->info.damageSuperEffective |= COMBAT_GHOSTDAMAGE;
		mType->info.damageSuperEffective |= COMBAT_DARKDAMAGE;
		mType->info.damageNotVeryEffective |= COMBAT_FIGHTINGDAMAGE;
		mType->info.damageNotVeryEffective |= COMBAT_PSYCHICDAMAGE;
	}
	if (mType->info.firstType == TYPE_BUG || mType->info.secondType == TYPE_BUG) {
		mType->info.damageSuperEffective |= COMBAT_FIREDAMAGE;
		mType->info.damageSuperEffective |= COMBAT_FLYINGDAMAGE;
		mType->info.damageSuperEffective |= COMBAT_ROCKDAMAGE;
		mType->info.damageNotVeryEffective |= COMBAT_FIGHTINGDAMAGE;
		mType->info.damageNotVeryEffective |= COMBAT_GRASSDAMAGE;
		mType->info.damageNotVeryEffective |= COMBAT_GROUNDDAMAGE;
	}
	if (mType->info.firstType == TYPE_ROCK || mType->info.secondType == TYPE_ROCK) {
		mType->info.damageSuperEffective |= COMBAT_WATERDAMAGE;
		mType->info.damageSuperEffective |= COMBAT_GRASSDAMAGE;
		mType->info.damageSuperEffective |= COMBAT_FIGHTINGDAMAGE;
		mType->info.damageSuperEffective |= COMBAT_GROUNDDAMAGE;
		mType->info.damageSuperEffective |= COMBAT_STEELDAMAGE;
		mType->info.damageNotVeryEffective |= COMBAT_NORMALDAMAGE;
		mType->info.damageNotVeryEffective |= COMBAT_FIREDAMAGE;
		mType->info.damageNotVeryEffective |= COMBAT_POISONDAMAGE;
		mType->info.damageNotVeryEffective |= COMBAT_FLYINGDAMAGE;
	}
	if (mType->info.firstType == TYPE_DRAGON || mType->info.secondType == TYPE_DRAGON) {
		mType->info.damageSuperEffective |= COMBAT_ICEDAMAGE;
		mType->info.damageSuperEffective |= COMBAT_DRAGONDAMAGE;
		mType->info.damageSuperEffective |= COMBAT_FAIRYDAMAGE;
		mType->info.damageNotVeryEffective |= COMBAT_GRASSDAMAGE;
		mType->info.damageNotVeryEffective |= COMBAT_FIREDAMAGE;
		mType->info.damageNotVeryEffective |= COMBAT_WATERDAMAGE;
		mType->info.damageNotVeryEffective |= COMBAT_ELECTRICDAMAGE;
	}
	if (mType->info.firstType == TYPE_FIRE || mType->info.secondType == TYPE_FIRE) {
		mType->info.damageSuperEffective |= COMBAT_WATERDAMAGE;
		mType->info.damageSuperEffective |= COMBAT_GROUNDDAMAGE;
		mType->info.damageSuperEffective |= COMBAT_ROCKDAMAGE;
		mType->info.damageNotVeryEffective |= COMBAT_FIREDAMAGE;
		mType->info.damageNotVeryEffective |= COMBAT_GRASSDAMAGE;
		mType->info.damageNotVeryEffective |= COMBAT_ICEDAMAGE;
		mType->info.damageNotVeryEffective |= COMBAT_BUGDAMAGE;
		mType->info.damageNotVeryEffective |= COMBAT_STEELDAMAGE;
		mType->info.damageNotVeryEffective |= COMBAT_FAIRYDAMAGE;
	}
	
	// custom immunities
	if ((node = pokemonNode.child("immunities"))) {
		for (auto immunityNode : node.children()) {
			if ((attr = immunityNode.attribute("name"))) {
				std::string tmpStrValue = asLowerCaseString(attr.as_string());
				if (tmpStrValue == "physical") {
					mType->info.damageImmunities |= COMBAT_PHYSICALDAMAGE;
					mType->info.conditionImmunities |= CONDITION_BLEEDING;
				} else if (tmpStrValue == "electric") {
					mType->info.damageImmunities |= COMBAT_ELECTRICDAMAGE;
					mType->info.conditionImmunities |= CONDITION_PARALYZE;
				} else if (tmpStrValue == "fire") {
					mType->info.damageImmunities |= COMBAT_FIREDAMAGE;
					mType->info.conditionImmunities |= CONDITION_FIRE;
				} else if (tmpStrValue == "grass") {
					mType->info.damageImmunities |= COMBAT_GRASSDAMAGE;
					mType->info.conditionImmunities |= CONDITION_SLEEP;
				} else if (tmpStrValue == "water") {
					mType->info.damageImmunities |= COMBAT_WATERDAMAGE;
					mType->info.conditionImmunities |= CONDITION_DROWN;
				} else if (tmpStrValue == "ice") {
					mType->info.damageImmunities |= COMBAT_ICEDAMAGE;
					mType->info.conditionImmunities |= CONDITION_FREEZING;
				} else if (tmpStrValue == "bug") {
					mType->info.damageImmunities |= COMBAT_BUGDAMAGE;
				} else if (tmpStrValue == "posion") {
					mType->info.damageImmunities |= COMBAT_POISONDAMAGE;
					mType->info.conditionImmunities |= CONDITION_POISON;
				} else if (tmpStrValue == "lifedrain") {
					mType->info.damageImmunities |= COMBAT_LIFEDRAIN;
				} else if (tmpStrValue == "paralyze") {
					mType->info.conditionImmunities |= CONDITION_PARALYZE;
				} else if (tmpStrValue == "outfit") {
					mType->info.conditionImmunities |= CONDITION_OUTFIT;
				} else if (tmpStrValue == "drunk") {
					mType->info.conditionImmunities |= CONDITION_CONFUSION;
				} else if (tmpStrValue == "invisible" || tmpStrValue == "invisibility") {
					mType->info.conditionImmunities |= CONDITION_INVISIBLE;
				} else if (tmpStrValue == "bleed") {
					mType->info.conditionImmunities |= CONDITION_BLEEDING;
				} else {
					std::cout << "[Warning - Pokemons::loadPokemon] Unknown immunity name " << attr.as_string() << ". " << file << std::endl;
				}
			} else if ((attr = immunityNode.attribute("physical"))) {
				if (attr.as_bool()) {
					mType->info.damageImmunities |= COMBAT_PHYSICALDAMAGE;
					mType->info.conditionImmunities |= CONDITION_BLEEDING;
				}
			} else if ((attr = immunityNode.attribute("energy"))) {
				if (attr.as_bool()) {
					mType->info.damageImmunities |= COMBAT_ELECTRICDAMAGE;
					mType->info.conditionImmunities |= CONDITION_ENERGY;
				}
			} else if ((attr = immunityNode.attribute("fire"))) {
				if (attr.as_bool()) {
					mType->info.damageImmunities |= COMBAT_FIREDAMAGE;
					mType->info.conditionImmunities |= CONDITION_FIRE;
				}
			} else if ((attr = immunityNode.attribute("poison")) || (attr = immunityNode.attribute("earth"))) {
				if (attr.as_bool()) {
					mType->info.damageImmunities |= COMBAT_GRASSDAMAGE;
					mType->info.conditionImmunities |= CONDITION_POISON;
				}
			} else if ((attr = immunityNode.attribute("drown"))) {
				if (attr.as_bool()) {
					mType->info.damageImmunities |= COMBAT_WATERDAMAGE;
					mType->info.conditionImmunities |= CONDITION_DROWN;
				}
			} else if ((attr = immunityNode.attribute("ice"))) {
				if (attr.as_bool()) {
					mType->info.damageImmunities |= COMBAT_ICEDAMAGE;
					mType->info.conditionImmunities |= CONDITION_FREEZING;
				}
			} else if ((attr = immunityNode.attribute("holy"))) {
				if (attr.as_bool()) {
					mType->info.damageImmunities |= COMBAT_BUGDAMAGE;
					mType->info.conditionImmunities |= CONDITION_DAZZLED;
				}
			} else if ((attr = immunityNode.attribute("death"))) {
				if (attr.as_bool()) {
					mType->info.damageImmunities |= COMBAT_DARKDAMAGE;
					mType->info.conditionImmunities |= CONDITION_CURSED;
				}
			} else if ((attr = immunityNode.attribute("lifedrain"))) {
				if (attr.as_bool()) {
					mType->info.damageImmunities |= COMBAT_LIFEDRAIN;
				}
			} else if ((attr = immunityNode.attribute("paralyze"))) {
				if (attr.as_bool()) {
					mType->info.conditionImmunities |= CONDITION_PARALYZE;
				}
			} else if ((attr = immunityNode.attribute("outfit"))) {
				if (attr.as_bool()) {
					mType->info.conditionImmunities |= CONDITION_OUTFIT;
				}
			} else if ((attr = immunityNode.attribute("bleed"))) {
				if (attr.as_bool()) {
					mType->info.conditionImmunities |= CONDITION_BLEEDING;
				}
			} else if ((attr = immunityNode.attribute("drunk"))) {
				if (attr.as_bool()) {
					mType->info.conditionImmunities |= CONDITION_CONFUSION;
				}
			} else if ((attr = immunityNode.attribute("invisible")) || (attr = immunityNode.attribute("invisibility"))) {
				if (attr.as_bool()) {
					mType->info.conditionImmunities |= CONDITION_INVISIBLE;
				}
			} else {
				std::cout << "[Warning - Pokemons::loadPokemon] Unknown immunity. " << file << std::endl;
			}
		}
	}

	if ((node = pokemonNode.child("shiny"))) {
		pugi::xml_node auxNode;

		if ((attr = node.attribute("chance"))) {
			mType->info.shiny.chance = pugi::cast<double>(attr.value());
		} else {
			std::cout << "[Warning - Pokemons::loadPokemon] Missing shiny chance. " << file << std::endl;
		}

		if ((auxNode = node.child("look"))) {
			if ((attr = auxNode.attribute("type"))) {
				mType->info.shiny.outfit.lookType = pugi::cast<uint16_t>(attr.value());
			} else {
				std::cout << "[Warning - Pokemons::loadPokemon] Missing shiny look type. " << file << std::endl;
			}

			if ((attr = auxNode.attribute("corpse"))) {
				mType->info.shiny.corpse = pugi::cast<uint16_t>(attr.value());
			} else {
				std::cout << "[Warning - Pokemons::loadPokemon] Missing shiny look corpse. " << file << std::endl;
			}
		} else {
			std::cout << "[Warning - Pokemons::loadPokemon] Missing shiny look. " << file << std::endl;
		}

		if ((auxNode = node.child("portrait"))) {
			if ((attr = auxNode.attribute("id"))) {
				mType->info.shiny.portrait = pugi::cast<uint16_t>(attr.value());
			} else {
				std::cout << "[Warning - Pokemons::loadPokemon] Missing shiny portrait id. " << file << std::endl;
			}
		} else {
			std::cout << "[Warning - Pokemons::loadPokemon] Missing shiny portrait. " << file << std::endl;
		}

		if ((auxNode = node.child("icon"))) {
			if ((attr = auxNode.attribute("charged"))) {
				mType->info.shiny.iconCharged = pugi::cast<uint16_t>(attr.value());
			} else {
				std::cout << "[Warning - Pokemons::loadPokemon] Missing shiny icon charged. " << file << std::endl;
			}

			if ((attr = auxNode.attribute("discharged"))) {
				mType->info.shiny.iconDischarged = pugi::cast<uint16_t>(attr.value());
			} else {
				std::cout << "[Warning - Pokemons::loadPokemon] Missing shiny icon discharged. " << file << std::endl;
			}
		} else {
			std::cout << "[Warning - Pokemons::loadPokemon] Missing shiny icon. " << file << std::endl;
		}
	}

	if ((node = pokemonNode.child("evolutions"))) {
		pugi::xml_node auxNode;

		for (auto evolutionNode : node.children()) {
			EvolutionBlock_t evolutionBlock;

			if ((attr = evolutionNode.attribute("to"))) {
				evolutionBlock.to = pugi::cast<std::string>(attr.value());
			} else {
				std::cout << "[Warning - Pokemons::loadPokemon] Evolution to is missing. " << file << std::endl;
			}

			if ((attr = evolutionNode.attribute("at"))) {
				std::string at = pugi::cast<std::string>(attr.value());

				if (at == "day") {
					evolutionBlock.at = 2;
				} else if (at == "night") {
					evolutionBlock.at = 1;
				} else if (at == "anytime") {
					evolutionBlock.at = 0;
				} else {
					std::cout << "[Warning - Pokemons::loadPokemon] Unknown evolution at value. " << file << std::endl;
				}
			} else {
				std::cout << "[Warning - Pokemons::loadPokemon] Evolution at is missing. " << file << std::endl;
			}
			
			if ((attr = evolutionNode.attribute("stone"))) {
				evolutionBlock.stone = pugi::cast<uint16_t>(attr.value());
			} else {
				std::cout << "[Warning - Pokemons::loadPokemon] Evolution stone is missing. " << file << std::endl;
			}

			mType->info.canEvolve = true;
			mType->info.evolutions.emplace_back(std::move(evolutionBlock));
		}
	}

	if ((node = pokemonNode.child("voices"))) {
		if ((attr = node.attribute("speed")) || (attr = node.attribute("interval"))) {
			mType->info.yellSpeedTicks = pugi::cast<uint32_t>(attr.value());
		} else {
			std::cout << "[Warning - Pokemons::loadPokemon] Missing voices speed. " << file << std::endl;
		}

		if ((attr = node.attribute("chance"))) {
			mType->info.yellChance = pugi::cast<uint32_t>(attr.value());
		} else {
			std::cout << "[Warning - Pokemons::loadPokemon] Missing voices chance. " << file << std::endl;
		}

		for (auto voiceNode : node.children()) {
			voiceBlock_t vb;
			if ((attr = voiceNode.attribute("sentence"))) {
				vb.text = attr.as_string();
			} else {
				std::cout << "[Warning - Pokemons::loadPokemon] Missing voice sentence. " << file << std::endl;
			}

			if ((attr = voiceNode.attribute("sound"))) {
				SoundEffectClasses soundEffect = getSoundEffect(asLowerCaseString(attr.as_string()));
				if (soundEffect) {
					vb.sound = soundEffect;
				} else {
					std::cout << "[Warning - Pokemons::loadPokemon] Unknown sound effect: " << attr.as_string() << std::endl;
				}
			}

			if ((attr = voiceNode.attribute("yell"))) {
				vb.yellText = attr.as_bool();
			} else {
				vb.yellText = false;
			}
			mType->info.voiceVector.emplace_back(vb);
		}
	}

	if ((node = pokemonNode.child("loot"))) {
		for (auto lootNode : node.children()) {
			LootBlock lootBlock;
			if (loadLootItem(lootNode, lootBlock)) {
				mType->info.lootItems.emplace_back(std::move(lootBlock));
			} else {
				std::cout << "[Warning - Pokemons::loadPokemon] Cant load loot. " << file << std::endl;
			}
		}
	}

	if ((node = pokemonNode.child("elements"))) {
		for (auto elementNode : node.children()) {
			if ((attr = elementNode.attribute("physicalPercent"))) {
				mType->info.elementMap[COMBAT_PHYSICALDAMAGE] = pugi::cast<int32_t>(attr.value());
			} else if ((attr = elementNode.attribute("icePercent"))) {
				mType->info.elementMap[COMBAT_ICEDAMAGE] = pugi::cast<int32_t>(attr.value());
			} else if ((attr = elementNode.attribute("poisonPercent")) || (attr = elementNode.attribute("earthPercent"))) {
				mType->info.elementMap[COMBAT_GRASSDAMAGE] = pugi::cast<int32_t>(attr.value());
			} else if ((attr = elementNode.attribute("firePercent"))) {
				mType->info.elementMap[COMBAT_FIREDAMAGE] = pugi::cast<int32_t>(attr.value());
			} else if ((attr = elementNode.attribute("energyPercent"))) {
				mType->info.elementMap[COMBAT_ELECTRICDAMAGE] = pugi::cast<int32_t>(attr.value());
			} else if ((attr = elementNode.attribute("holyPercent"))) {
				mType->info.elementMap[COMBAT_BUGDAMAGE] = pugi::cast<int32_t>(attr.value());
			} else if ((attr = elementNode.attribute("deathPercent"))) {
				mType->info.elementMap[COMBAT_DARKDAMAGE] = pugi::cast<int32_t>(attr.value());
			} else if ((attr = elementNode.attribute("drownPercent"))) {
				mType->info.elementMap[COMBAT_WATERDAMAGE] = pugi::cast<int32_t>(attr.value());
			} else if ((attr = elementNode.attribute("lifedrainPercent"))) {
				mType->info.elementMap[COMBAT_LIFEDRAIN] = pugi::cast<int32_t>(attr.value());
			} else {
				std::cout << "[Warning - Pokemons::loadPokemon] Unknown element percent. " << file << std::endl;
			}
		}
	}

	if ((node = pokemonNode.child("summons"))) {
		if ((attr = node.attribute("maxSummons"))) {
			mType->info.maxSummons = std::min<uint32_t>(pugi::cast<uint32_t>(attr.value()), 100);
		} else {
			std::cout << "[Warning - Pokemons::loadPokemon] Missing summons maxSummons. " << file << std::endl;
		}

		for (auto summonNode : node.children()) {
			int32_t chance = 100;
			int32_t speed = 1000;
			int32_t max = mType->info.maxSummons;
			bool force = false;

			if ((attr = summonNode.attribute("speed")) || (attr = summonNode.attribute("interval"))) {
				speed = std::max<int32_t>(1, pugi::cast<int32_t>(attr.value()));
			}

			if ((attr = summonNode.attribute("chance"))) {
				chance = pugi::cast<int32_t>(attr.value());
			}

			if ((attr = summonNode.attribute("max"))) {
				max = pugi::cast<uint32_t>(attr.value());
			}

			if ((attr = summonNode.attribute("force"))) {
				force = attr.as_bool();
			}

			if ((attr = summonNode.attribute("name"))) {
				summonBlock_t sb;
				sb.name = attr.as_string();
				sb.speed = speed;
				sb.chance = chance;
				sb.max = max;
				sb.force = force;
				mType->info.summons.emplace_back(sb);
			} else {
				std::cout << "[Warning - Pokemons::loadPokemon] Missing summon name. " << file << std::endl;
			}
		}
	}

	if ((node = pokemonNode.child("script"))) {
		for (auto eventNode : node.children()) {
			if ((attr = eventNode.attribute("name"))) {
				mType->info.scripts.emplace_back(attr.as_string());
			} else {
				std::cout << "[Warning - Pokemons::loadPokemon] Missing name for script event. " << file << std::endl;
			}
		}
	}

	mType->info.summons.shrink_to_fit();
	mType->info.lootItems.shrink_to_fit();
	mType->info.voiceVector.shrink_to_fit();
	mType->info.scripts.shrink_to_fit();
	mType->info.evolutions.shrink_to_fit();
	return mType;
}

bool Pokemons::loadLootItem(const pugi::xml_node& node, LootBlock& lootBlock)
{
	pugi::xml_attribute attr;
	if ((attr = node.attribute("id"))) {
		lootBlock.id = pugi::cast<int32_t>(attr.value());
	} else if ((attr = node.attribute("name"))) {
		auto name = attr.as_string();
		auto ids = Item::items.nameToItems.equal_range(asLowerCaseString(name));

		if (ids.first == Item::items.nameToItems.cend()) {
			std::cout << "[Warning - Pokemons::loadPokemon] Unknown loot item \"" << name << "\". " << std::endl;
			return false;
		}

		uint32_t id = ids.first->second;

		if (std::next(ids.first) != ids.second) {
			std::cout << "[Warning - Pokemons::loadPokemon] Non-unique loot item \"" << name << "\". " << std::endl;
			return false;
		}

		lootBlock.id = id;
	}

	if (lootBlock.id == 0) {
		return false;
	}

	if ((attr = node.attribute("countmax"))) {
		lootBlock.countmax = std::max<int32_t>(1, pugi::cast<int32_t>(attr.value()));
	} else {
		lootBlock.countmax = 1;
	}

	if ((attr = node.attribute("chance")) || (attr = node.attribute("chance1"))) {
		lootBlock.chance = std::min<int32_t>(MAX_LOOTCHANCE, pugi::cast<int32_t>(attr.value()));
	} else {
		lootBlock.chance = MAX_LOOTCHANCE;
	}

	if (Item::items[lootBlock.id].isContainer()) {
		loadLootContainer(node, lootBlock);
	}

	//optional
	if ((attr = node.attribute("subtype"))) {
		lootBlock.subType = pugi::cast<int32_t>(attr.value());
	} else {
		uint32_t charges = Item::items[lootBlock.id].charges;
		if (charges != 0) {
			lootBlock.subType = charges;
		}
	}

	if ((attr = node.attribute("actionId"))) {
		lootBlock.actionId = pugi::cast<int32_t>(attr.value());
	}

	if ((attr = node.attribute("text"))) {
		lootBlock.text = attr.as_string();
	}
	return true;
}

void Pokemons::loadLootContainer(const pugi::xml_node& node, LootBlock& lBlock)
{
	for (auto subNode : node.children()) {
		LootBlock lootBlock;
		if (loadLootItem(subNode, lootBlock)) {
			lBlock.childLoot.emplace_back(std::move(lootBlock));
		}
	}
}

PokemonType* Pokemons::getPokemonType(const std::string& name)
{
	std::string lowerCaseName = asLowerCaseString(name);

	auto it = pokemons.find(lowerCaseName);
	if (it == pokemons.end()) {
		auto it2 = unloadedPokemons.find(lowerCaseName);
		if (it2 == unloadedPokemons.end()) {
			return nullptr;
		}

		return loadPokemon(it2->second, name);
	}
	return &it->second;
}
