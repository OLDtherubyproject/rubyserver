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

#include "tools.h"
#include "configmanager.h"
#include "boost/date_time/posix_time/posix_time.hpp"

extern ConfigManager g_config;

void printXMLError(const std::string& where, const std::string& fileName, const pugi::xml_parse_result& result)
{
	std::cout << '[' << where << "] Failed to load " << fileName << ": " << result.description() << std::endl;

	FILE* file = fopen(fileName.c_str(), "rb");
	if (!file) {
		return;
	}

	char buffer[32768];
	uint32_t currentLine = 1;
	std::string line;

	size_t offset = static_cast<size_t>(result.offset);
	size_t lineOffsetPosition = 0;
	size_t index = 0;
	size_t bytes;
	do {
		bytes = fread(buffer, 1, 32768, file);
		for (size_t i = 0; i < bytes; ++i) {
			char ch = buffer[i];
			if (ch == '\n') {
				if ((index + i) >= offset) {
					lineOffsetPosition = line.length() - ((index + i) - offset);
					bytes = 0;
					break;
				}
				++currentLine;
				line.clear();
			} else {
				line.push_back(ch);
			}
		}
		index += bytes;
	} while (bytes == 32768);
	fclose(file);

	std::cout << "Line " << currentLine << ':' << std::endl;
	std::cout << line << std::endl;
	for (size_t i = 0; i < lineOffsetPosition; i++) {
		if (line[i] == '\t') {
			std::cout << '\t';
		} else {
			std::cout << ' ';
		}
	}
	std::cout << '^' << std::endl;
}

static uint32_t circularShift(int bits, uint32_t value)
{
	return (value << bits) | (value >> (32 - bits));
}

static void processSHA1MessageBlock(const uint8_t* messageBlock, uint32_t* H)
{
	uint32_t W[80];
	for (int i = 0; i < 16; ++i) {
		const size_t offset = i << 2;
		W[i] = messageBlock[offset] << 24 | messageBlock[offset + 1] << 16 | messageBlock[offset + 2] << 8 | messageBlock[offset + 3];
	}

	for (int i = 16; i < 80; ++i) {
		W[i] = circularShift(1, W[i - 3] ^ W[i - 8] ^ W[i - 14] ^ W[i - 16]);
	}

	uint32_t A = H[0], B = H[1], C = H[2], D = H[3], E = H[4];

	for (int i = 0; i < 20; ++i) {
		const uint32_t tmp = circularShift(5, A) + ((B & C) | ((~B) & D)) + E + W[i] + 0x5A827999;
		E = D; D = C; C = circularShift(30, B); B = A; A = tmp;
	}

	for (int i = 20; i < 40; ++i) {
		const uint32_t tmp = circularShift(5, A) + (B ^ C ^ D) + E + W[i] + 0x6ED9EBA1;
		E = D; D = C; C = circularShift(30, B); B = A; A = tmp;
	}

	for (int i = 40; i < 60; ++i) {
		const uint32_t tmp = circularShift(5, A) + ((B & C) | (B & D) | (C & D)) + E + W[i] + 0x8F1BBCDC;
		E = D; D = C; C = circularShift(30, B); B = A; A = tmp;
	}

	for (int i = 60; i < 80; ++i) {
		const uint32_t tmp = circularShift(5, A) + (B ^ C ^ D) + E + W[i] + 0xCA62C1D6;
		E = D; D = C; C = circularShift(30, B); B = A; A = tmp;
	}

	H[0] += A;
	H[1] += B;
	H[2] += C;
	H[3] += D;
	H[4] += E;
}

std::string transformToSHA1(const std::string& input)
{
	uint32_t H[] = {
		0x67452301,
		0xEFCDAB89,
		0x98BADCFE,
		0x10325476,
		0xC3D2E1F0
	};

	uint8_t messageBlock[64];
	size_t index = 0;

	uint32_t length_low = 0;
	uint32_t length_high = 0;
	for (char ch : input) {
		messageBlock[index++] = ch;

		length_low += 8;
		if (length_low == 0) {
			length_high++;
		}

		if (index == 64) {
			processSHA1MessageBlock(messageBlock, H);
			index = 0;
		}
	}

	messageBlock[index++] = 0x80;

	if (index > 56) {
		while (index < 64) {
			messageBlock[index++] = 0;
		}

		processSHA1MessageBlock(messageBlock, H);
		index = 0;
	}

	while (index < 56) {
		messageBlock[index++] = 0;
	}

	messageBlock[56] = length_high >> 24;
	messageBlock[57] = length_high >> 16;
	messageBlock[58] = length_high >> 8;
	messageBlock[59] = length_high;

	messageBlock[60] = length_low >> 24;
	messageBlock[61] = length_low >> 16;
	messageBlock[62] = length_low >> 8;
	messageBlock[63] = length_low;

	processSHA1MessageBlock(messageBlock, H);

	char hexstring[41];
	static const char hexDigits[] = {"0123456789abcdef"};
	for (int hashByte = 20; --hashByte >= 0;) {
		const uint8_t byte = H[hashByte >> 2] >> (((3 - hashByte) & 3) << 3);
		index = hashByte << 1;
		hexstring[index] = hexDigits[byte >> 4];
		hexstring[index + 1] = hexDigits[byte & 15];
	}
	return std::string(hexstring, 40);
}

std::string generateToken(const std::string& key, uint32_t ticks)
{
	// generate message from ticks
	std::string message(8, 0);
	for (uint8_t i = 8; --i; ticks >>= 8) {
		message[i] = static_cast<char>(ticks & 0xFF);
	}

	// hmac key pad generation
	std::string iKeyPad(64, 0x36), oKeyPad(64, 0x5C);
	for (uint8_t i = 0; i < key.length(); ++i) {
		iKeyPad[i] ^= key[i];
		oKeyPad[i] ^= key[i];
	}

	oKeyPad.reserve(84);

	// hmac concat inner pad with message
	iKeyPad.append(message);

	// hmac first pass
	message.assign(transformToSHA1(iKeyPad));

	// hmac concat outer pad with message, conversion from hex to int needed
	for (uint8_t i = 0; i < message.length(); i += 2) {
		oKeyPad.push_back(static_cast<char>(std::strtoul(message.substr(i, 2).c_str(), nullptr, 16)));
	}

	// hmac second pass
	message.assign(transformToSHA1(oKeyPad));

	// calculate hmac offset
	uint32_t offset = static_cast<uint32_t>(std::strtoul(message.substr(39, 1).c_str(), nullptr, 16) & 0xF);

	// get truncated hash
	uint32_t truncHash = static_cast<uint32_t>(std::strtoul(message.substr(2 * offset, 8).c_str(), nullptr, 16)) & 0x7FFFFFFF;
	message.assign(std::to_string(truncHash));

	// return only last AUTHENTICATOR_DIGITS (default 6) digits, also asserts exactly 6 digits
	uint32_t hashLen = message.length();
	message.assign(message.substr(hashLen - std::min(hashLen, AUTHENTICATOR_DIGITS)));
	message.insert(0, AUTHENTICATOR_DIGITS - std::min(hashLen, AUTHENTICATOR_DIGITS), '0');
	return message;
}

void replaceString(std::string& str, const std::string& sought, const std::string& replacement)
{
	size_t pos = 0;
	size_t start = 0;
	size_t soughtLen = sought.length();
	size_t replaceLen = replacement.length();

	while ((pos = str.find(sought, start)) != std::string::npos) {
		str = str.substr(0, pos) + replacement + str.substr(pos + soughtLen);
		start = pos + replaceLen;
	}
}

void trim_right(std::string& source, char t)
{
	source.erase(source.find_last_not_of(t) + 1);
}

void trim_left(std::string& source, char t)
{
	source.erase(0, source.find_first_not_of(t));
}

void toLowerCaseString(std::string& source)
{
	std::transform(source.begin(), source.end(), source.begin(), tolower);
}

std::string asLowerCaseString(std::string source)
{
	toLowerCaseString(source);
	return source;
}

std::string asUpperCaseString(std::string source)
{
	std::transform(source.begin(), source.end(), source.begin(), toupper);
	return source;
}

StringVector explodeString(const std::string& inString, const std::string& separator, int32_t limit/* = -1*/)
{
	StringVector returnVector;
	std::string::size_type start = 0, end = 0;

	while (--limit != -1 && (end = inString.find(separator, start)) != std::string::npos) {
		returnVector.push_back(inString.substr(start, end - start));
		start = end + separator.size();
	}

	returnVector.push_back(inString.substr(start));
	return returnVector;
}

IntegerVector vectorAtoi(const StringVector& stringVector)
{
	IntegerVector returnVector;
	for (const auto& string : stringVector) {
		returnVector.push_back(std::stoi(string));
	}
	return returnVector;
}

std::mt19937& getRandomGenerator()
{
	static std::random_device rd;
	static std::mt19937 generator(rd());
	return generator;
}

int32_t uniform_random(int32_t minNumber, int32_t maxNumber)
{
	static std::uniform_int_distribution<int32_t> uniformRand;
	if (minNumber == maxNumber) {
		return minNumber;
	} else if (minNumber > maxNumber) {
		std::swap(minNumber, maxNumber);
	}
	return uniformRand(getRandomGenerator(), std::uniform_int_distribution<int32_t>::param_type(minNumber, maxNumber));
}

int32_t normal_random(int32_t minNumber, int32_t maxNumber)
{
	static std::normal_distribution<float> normalRand(0.5f, 0.25f);
	if (minNumber == maxNumber) {
		return minNumber;
	} else if (minNumber > maxNumber) {
		std::swap(minNumber, maxNumber);
	}

	int32_t increment;
	const int32_t diff = maxNumber - minNumber;
	const float v = normalRand(getRandomGenerator());
	if (v < 0.0) {
		increment = diff / 2;
	} else if (v > 1.0) {
		increment = (diff + 1) / 2;
	} else {
		increment = round(v * diff);
	}
	return minNumber + increment;
}

bool boolean_random(double probability/* = 0.5*/)
{
	static std::bernoulli_distribution booleanRand;
	return booleanRand(getRandomGenerator(), std::bernoulli_distribution::param_type(probability));
}

void trimString(std::string& str)
{
	str.erase(str.find_last_not_of(' ') + 1);
	str.erase(0, str.find_first_not_of(' '));
}

std::string convertIPToString(uint32_t ip)
{
	char buffer[17];

	int res = sprintf(buffer, "%u.%u.%u.%u", ip & 0xFF, (ip >> 8) & 0xFF, (ip >> 16) & 0xFF, (ip >> 24));
	if (res < 0) {
		return {};
	}

	return buffer;
}

std::string formatDate(time_t time)
{
	const tm* tms = localtime(&time);
	if (!tms) {
		return {};
	}

	char buffer[20];
	int res = sprintf(buffer, "%02d/%02d/%04d %02d:%02d:%02d", tms->tm_mday, tms->tm_mon + 1, tms->tm_year + 1900, tms->tm_hour, tms->tm_min, tms->tm_sec);
	if (res < 0) {
		return {};
	}
	return {buffer, 19};
}

std::string formatDateShort(time_t time)
{
	const tm* tms = localtime(&time);
	if (!tms) {
		return {};
	}

	char buffer[12];
	size_t res = strftime(buffer, 12, "%d %b %Y", tms);
	if (res == 0) {
		return {};
	}
	return {buffer, 11};
}

Direction getDirection(const std::string& string)
{
	Direction direction = DIRECTION_NORTH;

	if (string == "north" || string == "n" || string == "0") {
		direction = DIRECTION_NORTH;
	} else if (string == "east" || string == "e" || string == "1") {
		direction = DIRECTION_EAST;
	} else if (string == "south" || string == "s" || string == "2") {
		direction = DIRECTION_SOUTH;
	} else if (string == "west" || string == "w" || string == "3") {
		direction = DIRECTION_WEST;
	} else if (string == "southwest" || string == "south west" || string == "south-west" || string == "sw" || string == "4") {
		direction = DIRECTION_SOUTHWEST;
	} else if (string == "southeast" || string == "south east" || string == "south-east" || string == "se" || string == "5") {
		direction = DIRECTION_SOUTHEAST;
	} else if (string == "northwest" || string == "north west" || string == "north-west" || string == "nw" || string == "6") {
		direction = DIRECTION_NORTHWEST;
	} else if (string == "northeast" || string == "north east" || string == "north-east" || string == "ne" || string == "7") {
		direction = DIRECTION_NORTHEAST;
	}

	return direction;
}

Position getNextPosition(Direction direction, Position pos)
{
	switch (direction) {
		case DIRECTION_NORTH:
			pos.y--;
			break;

		case DIRECTION_SOUTH:
			pos.y++;
			break;

		case DIRECTION_WEST:
			pos.x--;
			break;

		case DIRECTION_EAST:
			pos.x++;
			break;

		case DIRECTION_SOUTHWEST:
			pos.x--;
			pos.y++;
			break;

		case DIRECTION_NORTHWEST:
			pos.x--;
			pos.y--;
			break;

		case DIRECTION_NORTHEAST:
			pos.x++;
			pos.y--;
			break;

		case DIRECTION_SOUTHEAST:
			pos.x++;
			pos.y++;
			break;

		default:
			break;
	}

	return pos;
}

Direction getDirectionTo(const Position& from, const Position& to)
{
	Direction dir;

	int32_t x_offset = Position::getOffsetX(from, to);
	if (x_offset < 0) {
		dir = DIRECTION_EAST;
		x_offset = std::abs(x_offset);
	} else {
		dir = DIRECTION_WEST;
	}

	int32_t y_offset = Position::getOffsetY(from, to);
	if (y_offset >= 0) {
		if (y_offset > x_offset) {
			dir = DIRECTION_NORTH;
		} else if (y_offset == x_offset) {
			if (dir == DIRECTION_EAST) {
				dir = DIRECTION_NORTHEAST;
			} else {
				dir = DIRECTION_NORTHWEST;
			}
		}
	} else {
		y_offset = std::abs(y_offset);
		if (y_offset > x_offset) {
			dir = DIRECTION_SOUTH;
		} else if (y_offset == x_offset) {
			if (dir == DIRECTION_EAST) {
				dir = DIRECTION_SOUTHEAST;
			} else {
				dir = DIRECTION_SOUTHWEST;
			}
		}
	}
	return dir;
}

using EffectNames = std::unordered_map<std::string, EffectClasses>;
using SoundEffectNames = std::unordered_map<std::string, SoundEffectClasses>;
using ShootTypeNames = std::unordered_map<std::string, ShootType_t>;
using CombatTypeNames = std::unordered_map<CombatType_t, std::string, std::hash<int32_t>>;
using GenderNames = std::unordered_map<std::string, Genders_t>;
using PokemonTypeNames = std::unordered_map<std::string, PokemonType_t>;

EffectNames effectNames = {
	{"redspark",			CONST_ME_DRAWBLOOD},
	{"bluebubble",			CONST_ME_LOSEENERGY},
	{"poff",				CONST_ME_POFF},
	{"yellowspark",			CONST_ME_BLOCKHIT},
	{"explosionarea",		CONST_ME_EXPLOSIONAREA},
	{"explosion",			CONST_ME_EXPLOSIONHIT},
	{"firearea",			CONST_ME_FIREAREA},
	{"yellowbubble",		CONST_ME_YELLOW_RINGS},
	{"greenbubble",			CONST_ME_GREEN_RINGS},
	{"blackspark",			CONST_ME_HITAREA},
	{"teleport",			CONST_ME_TELEPORT},
	{"energy",				CONST_ME_ENERGYHIT},
	{"blueshimmer",			CONST_ME_MAGIC_BLUE},
	{"redshimmer",			CONST_ME_MAGIC_RED},
	{"greenshimmer",		CONST_ME_MAGIC_GREEN},
	{"fire",				CONST_ME_HITBYFIRE},
	{"greenspark",			CONST_ME_HITBYPOISON},
	{"mortarea",			CONST_ME_MORTAREA},
	{"greennote",			CONST_ME_SOUND_GREEN},
	{"rednote",				CONST_ME_SOUND_RED},
	{"poison",				CONST_ME_POISONAREA},
	{"yellownote",			CONST_ME_SOUND_YELLOW},
	{"purplenote",			CONST_ME_SOUND_PURPLE},
	{"bluenote",			CONST_ME_SOUND_BLUE},
	{"whitenote",			CONST_ME_SOUND_WHITE},
	{"bubbles",				CONST_ME_BUBBLES},
	{"dice",				CONST_ME_CRAPS},
	{"giftwraps",			CONST_ME_GIFT_WRAPS},
	{"yellowfirework",		CONST_ME_FIREWORK_YELLOW},
	{"redfirework",			CONST_ME_FIREWORK_RED},
	{"bluefirework",		CONST_ME_FIREWORK_BLUE},
	{"stun",				CONST_ME_STUN},
	{"sleep",				CONST_ME_SLEEP},
	{"watercreature",		CONST_ME_WATERCREATURE},
	{"groundshaker",		CONST_ME_GROUNDSHAKER},
	{"hearts",				CONST_ME_HEARTS},
	{"fireattack",			CONST_ME_FIREATTACK},
	{"energyarea",			CONST_ME_ENERGYAREA},
	{"smallclouds",			CONST_ME_SMALLCLOUDS},
	{"holydamage",			CONST_ME_HOLYDAMAGE},
	{"bigclouds",			CONST_ME_BIGCLOUDS},
	{"icearea",				CONST_ME_ICEAREA},
	{"icetornado",			CONST_ME_ICETORNADO},
	{"iceattack",			CONST_ME_ICEATTACK},
	{"stones",				CONST_ME_STONES},
	{"smallplants",			CONST_ME_SMALLPLANTS},
	{"carniphila",			CONST_ME_CARNIPHILA},
	{"purpleenergy",		CONST_ME_PURPLEENERGY},
	{"yellowenergy",		CONST_ME_YELLOWENERGY},
	{"holyarea",			CONST_ME_HOLYAREA},
	{"bigplants",			CONST_ME_BIGPLANTS},
	{"cake",				CONST_ME_CAKE},
	{"giantice",			CONST_ME_GIANTICE},
	{"watersplash",			CONST_ME_WATERSPLASH},
	{"plantattack",			CONST_ME_PLANTATTACK},
	{"tutorialarrow",		CONST_ME_TUTORIALARROW},
	{"tutorialsquare",		CONST_ME_TUTORIALSQUARE},
	{"mirrorhorizontal",	CONST_ME_MIRRORHORIZONTAL},
	{"mirrorvertical",		CONST_ME_MIRRORVERTICAL},
	{"skullhorizontal",		CONST_ME_SKULLHORIZONTAL},
	{"skullvertical",		CONST_ME_SKULLVERTICAL},
	{"stepshorizontal",		CONST_ME_STEPSHORIZONTAL},
	{"bloodysteps",			CONST_ME_BLOODYSTEPS},
	{"stepsvertical",		CONST_ME_STEPSVERTICAL},
	{"yalaharighost",		CONST_ME_YALAHARIGHOST},
	{"bats",				CONST_ME_BATS},
	{"smoke",				CONST_ME_SMOKE},
	{"insects",				CONST_ME_INSECTS},
	{"dragonhead",			CONST_ME_DRAGONHEAD},
	{"thunder",				CONST_ME_THUNDER},
	{"confettihorizontal",	CONST_ME_CONFETTI_HORIZONTAL},
	{"confettivertical",	CONST_ME_CONFETTI_VERTICAL},
	{"blacksmoke",			CONST_ME_BLACKSMOKE},
	{"redsmoke",			CONST_ME_REDSMOKE},
	{"yellowsmoke",			CONST_ME_YELLOWSMOKE},
	{"greensmoke",			CONST_ME_GREENSMOKE},
	{"purplesmoke",			CONST_ME_PURPLESMOKE},
};

SoundEffectNames soundEffectNames = {
	{"catchsuccess",				CONST_SE_CATCHSUCCESS},
	{"saffronmusic",				CONST_SE_SAFFRONMUSIC},
	{"pokemonlowhp",				CONST_SE_POKEMONLOWHP},
	{"vinewhip",					CONST_SE_VINEWHIP},
	{"craps",						CONST_SE_CRAPS},
	{"teleport",					CONST_SE_TELEPORT},
	{"pokemoncenter",				CONST_SE_POKEMONCENTER},
	{"levelup",						CONST_SE_LEVELUP},
	{"pokemonhealed",				CONST_SE_POKEMONHEALED},
	{"newbarktown",					CONST_SE_NEWBARKTOWN},
	{"lyra",						CONST_SE_LYRA},
	{"elmpokemonlab",				CONST_SE_ELMPOKEMONLAB},
	{"obtainedakeyitem",			CONST_SE_OBTAINEDAKEYITEM},
	{"route29",						CONST_SE_ROUTE29},
	{"battlewildjohto",				CONST_SE_BATTLEWILDJOHTO},
	{"victorywildpokemon",			CONST_SE_VICTORYWILDPOKEMON},
	{"cherrygrovecity",				CONST_SE_CHERRYGROVECITY},
	{"battletrainerjohto",			CONST_SE_BATTLETRAINERJOHTO},
	{"victorytrainerjohto",			CONST_SE_VICTORYTRAINERJOHTO},
	{"route30",						CONST_SE_ROUTE30},
	{"pokedexevaluationnogood",		CONST_SE_POKEDEXEVALUATIONNOGOOD},
	{"violetcity",					CONST_SE_VIOLETCITY},
	{"sprouttower",					CONST_SE_SPROUTTOWER},
	{"pokemart",					CONST_SE_POKEMART},
	{"receivedpokemonegg",			CONST_SE_RECEIVEDPOKEMONEGG},
	{"kimonogirl",					CONST_SE_KIMONOGIRL},
	{"unioncave",					CONST_SE_UNIONCAVE},
	{"obtainedaitem",				CONST_SE_OBTAINEDAITEM},
	{"ruinsofalph",					CONST_SE_RUINSOFALPH},
	{"pokegearradiounown",			CONST_SE_POKEGEARRADIOUNOWN},
	{"pokedexevaluationyouareony",	CONST_SE_POKEDEXEVALUATIONYOUAREONY},
	{"azaleatown",					CONST_SE_AZALEATOWN},
	{"battleteamrocket",			CONST_SE_BATTLETEAMROCKET},
	{"route34",						CONST_SE_ROUTE34},
	{"arivalappears",				CONST_SE_ARIVALAPPEARS},
	{"battlerival",					CONST_SE_BATTLERIVAL},
	{"evolution",					CONST_SE_EVOLUTION},
	{"congratulations",				CONST_SE_CONGRATULATIONS},
	{"goldenrodcity",				CONST_SE_GOLDENRODCITY},
	{"battlegymleaderjohto",		CONST_SE_BATTLEGYMLEADERJOHTO},
	{"victorygymleader",			CONST_SE_VICTORYGYMLEADER},
	{"receivedgymbadge",			CONST_SE_RECEIVEDGYMBADGE},
	{"pokegearradiopokemonchannel",	CONST_SE_POKEGEARRADIOPOKEMONCHANNEL},
	{"pokegearradiobuenapassword",	CONST_SE_POKEGEARRADIOBUENAPASSWORD},
	{"receivedtm",					CONST_SE_RECEIVEDTM},
	{"goldenrodgamecorner",			CONST_SE_GOLDENRODGAMECORNER},
	{"youareawinner",				CONST_SE_YOUAREAWINNER},
	{"obtainedanaccessory",			CONST_SE_OBTAINEDANACCESSORY},
	{"globalterminal",				CONST_SE_GLOBALTERMINAL},
	{"waterdrop",					CONST_SE_WATERDROP},
};

ShootTypeNames shootTypeNames = {
	{"pokeball",			CONST_ANI_POKEBALL},
	{"greatball",			CONST_ANI_GREATBALL},
	{"ultraball",			CONST_ANI_ULTRABALL},
	{"saffariball",			CONST_ANI_SAFFARIBALL},
	{"masterball",			CONST_ANI_MASTERBALL},
	{"poisonarrow",			CONST_ANI_POISONARROW},
	{"burstarrow",			CONST_ANI_BURSTARROW},
	{"throwingstar",		CONST_ANI_THROWINGSTAR},
	{"throwingknife",		CONST_ANI_THROWINGKNIFE},
	{"smallstone",			CONST_ANI_SMALLSTONE},
	{"death",				CONST_ANI_DEATH},
	{"largerock",			CONST_ANI_LARGEROCK},
	{"snowball",			CONST_ANI_SNOWBALL},
	{"powerbolt",			CONST_ANI_POWERBOLT},
	{"poison",				CONST_ANI_POISON},
	{"infernalbolt",		CONST_ANI_INFERNALBOLT},
	{"huntingspear",		CONST_ANI_HUNTINGSPEAR},
	{"enchantedspear",		CONST_ANI_ENCHANTEDSPEAR},
	{"redstar",				CONST_ANI_REDSTAR},
	{"greenstar",			CONST_ANI_GREENSTAR},
	{"royalspear",			CONST_ANI_ROYALSPEAR},
	{"sniperarrow",			CONST_ANI_SNIPERARROW},
	{"onyxarrow",			CONST_ANI_ONYXARROW},
	{"piercingbolt",		CONST_ANI_PIERCINGBOLT},
	{"whirlwindsword",		CONST_ANI_WHIRLWINDSWORD},
	{"whirlwindaxe",		CONST_ANI_WHIRLWINDAXE},
	{"whirlwindclub",		CONST_ANI_WHIRLWINDCLUB},
	{"etherealspear",		CONST_ANI_ETHEREALSPEAR},
	{"ice",					CONST_ANI_ICE},
	{"earth",				CONST_ANI_EARTH},
	{"holy",				CONST_ANI_HOLY},
	{"suddendeath",			CONST_ANI_SUDDENDEATH},
	{"flasharrow",			CONST_ANI_FLASHARROW},
	{"flammingarrow",		CONST_ANI_FLAMMINGARROW},
	{"shiverarrow",			CONST_ANI_SHIVERARROW},
	{"energyball",			CONST_ANI_ENERGYBALL},
	{"smallice",			CONST_ANI_SMALLICE},
	{"smallholy",			CONST_ANI_SMALLHOLY},
	{"smallearth",			CONST_ANI_SMALLEARTH},
	{"eartharrow",			CONST_ANI_EARTHARROW},
	{"explosion",			CONST_ANI_EXPLOSION},
	{"cake",				CONST_ANI_CAKE},
	{"tarsalarrow",			CONST_ANI_TARSALARROW},
	{"vortexbolt",			CONST_ANI_VORTEXBOLT},
	{"prismaticbolt",		CONST_ANI_PRISMATICBOLT},
	{"crystallinearrow",	CONST_ANI_CRYSTALLINEARROW},
	{"drillbolt",			CONST_ANI_DRILLBOLT},
	{"envenomedarrow",		CONST_ANI_ENVENOMEDARROW},
	{"gloothspear",			CONST_ANI_GLOOTHSPEAR},
	{"simplearrow",			CONST_ANI_SIMPLEARROW},
};

CombatTypeNames combatTypeNames = {
	{COMBAT_PHYSICALDAMAGE, 		"physical"},
	{COMBAT_ELECTRICDAMAGE, 		"electric"},
	{COMBAT_GRASSDAMAGE, 			"grass"},
	{COMBAT_FIREDAMAGE, 			"fire"},
	{COMBAT_LIFEDRAIN, 				"lifedrain"},
	{COMBAT_HEALING, 				"healing"},
	{COMBAT_WATERDAMAGE, 			"water"},
	{COMBAT_ICEDAMAGE, 				"ice"},
	{COMBAT_BUGDAMAGE, 				"bug"},
	{COMBAT_DARKDAMAGE, 			"dark"},
	{COMBAT_NORMALDAMAGE,           "normal"},
	{COMBAT_FIGHTINGDAMAGE,         "fighting"},
	{COMBAT_FLYINGDAMAGE,           "flying"},
	{COMBAT_POISONDAMAGE,           "poison"},
	{COMBAT_GROUNDDAMAGE,           "ground"},
	{COMBAT_PSYCHICDAMAGE,          "psychic"},
	{COMBAT_ROCKDAMAGE,             "rock"},
	{COMBAT_GHOSTDAMAGE,            "ghost"},
	{COMBAT_STEELDAMAGE,            "steel"},
	{COMBAT_FAIRYDAMAGE,            "fairy"},
	{COMBAT_DRAGONDAMAGE,           "dragon"},
};

GenderNames genderNames = {
	{"none",		GENDER_NONE},
	{"male",		GENDER_MALE},
	{"female",		GENDER_FEMALE},
	{"undefined",	GENDER_UNDEFINED},
};

PokemonTypeNames pokemonTypes = {
	{"none",		TYPE_NONE},
	{"normal",		TYPE_NORMAL},
	{"fire",		TYPE_FIRE},
	{"fighting",	TYPE_FIGHTING},
	{"water",		TYPE_WATER},
	{"flying",		TYPE_FLYING},
	{"grass",		TYPE_GRASS},
	{"electric",	TYPE_ELECTRIC},
	{"poison",		TYPE_POISON},
	{"ground",		TYPE_GROUND},
	{"psychic",		TYPE_PSYCHIC},
	{"rock",		TYPE_ROCK},
	{"ice",			TYPE_ICE},
	{"bug",			TYPE_BUG},
	{"dragon",		TYPE_DRAGON},
	{"ghost",		TYPE_GHOST},
	{"dark",		TYPE_DARK},
	{"steel",		TYPE_STEEL},
	{"fairy",		TYPE_FAIRY},
};

EffectClasses getEffect(const std::string& strValue)
{
	auto effect = effectNames.find(strValue);
	if (effect != effectNames.end()) {
		return effect->second;
	}
	return CONST_ME_NONE;
}

SoundEffectClasses getSoundEffect(const std::string& strValue)
{
	auto soundEffect = soundEffectNames.find(strValue);
	if (soundEffect != soundEffectNames.end()) {
		return soundEffect->second;
	}
	return CONST_SE_NONE;
}

ShootType_t getShootType(const std::string& strValue)
{
	auto shootType = shootTypeNames.find(strValue);
	if (shootType != shootTypeNames.end()) {
		return shootType->second;
	}
	return CONST_ANI_NONE;
}

std::string getCombatName(CombatType_t combatType)
{
	auto combatName = combatTypeNames.find(combatType);
	if (combatName != combatTypeNames.end()) {
		return combatName->second;
	}
	return "unknown";
}

Genders_t getGenderType(const std::string& strValue)
{
	auto genderType = genderNames.find(strValue);
	if (genderType != genderNames.end()) {
		return genderType->second;
	}
	return GENDER_NONE;
}

PokemonType_t getPokemonElementType(const std::string& strValue)
{
	auto pokemonType = pokemonTypes.find(strValue);
	if (pokemonType != pokemonTypes.end()) {
		return pokemonType->second;
	}
	return TYPE_NONE;
}

std::string getSkillName(uint8_t skillid)
{
	switch (skillid) {
		case SKILL_FIST:
			return "fist fighting";

		case SKILL_CLUB:
			return "club fighting";

		case SKILL_SWORD:
			return "sword fighting";

		case SKILL_AXE:
			return "axe fighting";

		case SKILL_DISTANCE:
			return "distance fighting";

		case SKILL_SHIELD:
			return "shielding";

		case SKILL_FISHING:
			return "fishing";

		case SKILL_LEVEL:
			return "level";

		default:
			return "unknown";
	}
}

uint32_t adlerChecksum(const uint8_t* data, size_t length)
{
	if (length > NETWORKMESSAGE_MAXSIZE) {
		return 0;
	}

	const uint16_t adler = 65521;

	uint32_t a = 1, b = 0;

	while (length > 0) {
		size_t tmp = length > 5552 ? 5552 : length;
		length -= tmp;

		do {
			a += *data++;
			b += a;
		} while (--tmp);

		a %= adler;
		b %= adler;
	}

	return (b << 16) | a;
}

std::string ucfirst(std::string str)
{
	for (char& i : str) {
		if (i != ' ') {
			i = toupper(i);
			break;
		}
	}
	return str;
}

std::string ucwords(std::string str)
{
	size_t strLength = str.length();
	if (strLength == 0) {
		return str;
	}

	str[0] = toupper(str.front());
	for (size_t i = 1; i < strLength; ++i) {
		if (str[i - 1] == ' ') {
			str[i] = toupper(str[i]);
		}
	}

	return str;
}

bool booleanString(const std::string& str)
{
	if (str.empty()) {
		return false;
	}

	char ch = tolower(str.front());
	return ch != 'f' && ch != 'n' && ch != '0';
}

size_t combatTypeToIndex(CombatType_t combatType)
{
	switch (combatType) {
		case COMBAT_PHYSICALDAMAGE:
			return 0;
		case COMBAT_ELECTRICDAMAGE:
			return 1;
		case COMBAT_GRASSDAMAGE:
			return 2;
		case COMBAT_FIREDAMAGE:
			return 3;
		case COMBAT_LIFEDRAIN:
			return 4;
		case COMBAT_HEALING:
			return 5;
		case COMBAT_WATERDAMAGE:
			return 6;
		case COMBAT_ICEDAMAGE:
			return 7;
		case COMBAT_BUGDAMAGE:
			return 8;
		case COMBAT_DARKDAMAGE:
			return 9;
		case COMBAT_NORMALDAMAGE:
			return 10;
		case COMBAT_FIGHTINGDAMAGE:
			return 11;
		case COMBAT_FLYINGDAMAGE:
			return 12;
		case COMBAT_POISONDAMAGE:
			return 13;
		case COMBAT_GROUNDDAMAGE:
			return 14;
		case COMBAT_PSYCHICDAMAGE:
			return 15;
		case COMBAT_ROCKDAMAGE:
			return 16;
		case COMBAT_GHOSTDAMAGE:
			return 17;
		case COMBAT_STEELDAMAGE:
			return 18;
		case COMBAT_FAIRYDAMAGE:
			return 19;
		case COMBAT_DRAGONDAMAGE:
			return 20;
		default:
			return 0;
	}
}

CombatType_t indexToCombatType(size_t v)
{
	return static_cast<CombatType_t>(1 << v);
}

CombatType_t pokemonTypeToCombatType(PokemonType_t pokemonType)
{
	switch (pokemonType) {
		case TYPE_NORMAL:
			return COMBAT_NORMALDAMAGE;
		case TYPE_FIRE:
			return COMBAT_FIREDAMAGE;
		case TYPE_FIGHTING:
			return COMBAT_FIGHTINGDAMAGE;
		case TYPE_WATER:
			return COMBAT_WATERDAMAGE;
		case TYPE_FLYING:
			return COMBAT_FLYINGDAMAGE;
		case TYPE_GRASS:
			return COMBAT_GRASSDAMAGE;
		case TYPE_ELECTRIC:
			return COMBAT_ELECTRICDAMAGE;
		case TYPE_POISON:
			return COMBAT_POISONDAMAGE;
		case TYPE_GROUND:
			return COMBAT_GROUNDDAMAGE;
		case TYPE_PSYCHIC:
			return COMBAT_PSYCHICDAMAGE;
		case TYPE_ROCK:
			return COMBAT_ROCKDAMAGE;
		case TYPE_ICE:
			return COMBAT_ICEDAMAGE;
		case TYPE_BUG:
			return COMBAT_BUGDAMAGE;
		case TYPE_DRAGON:
			return COMBAT_DRAGONDAMAGE;
		case TYPE_GHOST:
			return COMBAT_GHOSTDAMAGE;
		case TYPE_DARK:
			return COMBAT_DARKDAMAGE;
		case TYPE_STEEL:
			return COMBAT_STEELDAMAGE;
		case TYPE_FAIRY:
			return COMBAT_FAIRYDAMAGE;
		default:
			return COMBAT_NONE;
	}
}

PokemonType_t combatTypeToPokemonType(CombatType_t combatType)
{
	switch (combatType) {
		case COMBAT_NORMALDAMAGE:
			return TYPE_NORMAL;
		case COMBAT_FIREDAMAGE:
			return TYPE_FIRE;
		case COMBAT_FIGHTINGDAMAGE:
			return TYPE_FIGHTING;
		case COMBAT_WATERDAMAGE:
			return TYPE_WATER;
		case COMBAT_FLYINGDAMAGE:
			return TYPE_FLYING;
		case COMBAT_GRASSDAMAGE:
			return TYPE_GRASS;
		case COMBAT_ELECTRICDAMAGE:
			return TYPE_ELECTRIC;
		case COMBAT_POISONDAMAGE:
			return TYPE_POISON;
		case COMBAT_GROUNDDAMAGE:
			return TYPE_GROUND;
		case COMBAT_PSYCHICDAMAGE:
			return TYPE_PSYCHIC;
		case COMBAT_ROCKDAMAGE:
			return TYPE_ROCK;
		case COMBAT_ICEDAMAGE:
			return TYPE_ICE;
		case COMBAT_BUGDAMAGE:
			return TYPE_BUG;
		case COMBAT_DRAGONDAMAGE:
			return TYPE_DRAGON;
		case COMBAT_GHOSTDAMAGE:
			return TYPE_GHOST;
		case COMBAT_DARKDAMAGE:
			return TYPE_DARK;
		case COMBAT_STEELDAMAGE:
			return TYPE_STEEL;
		case COMBAT_FAIRYDAMAGE:
			return TYPE_FAIRY;
		default:
			return TYPE_NONE;
	}
}

uint8_t serverFluidToClient(uint8_t serverFluid)
{
	uint8_t size = sizeof(clientToServerFluidMap) / sizeof(uint8_t);
	for (uint8_t i = 0; i < size; ++i) {
		if (clientToServerFluidMap[i] == serverFluid) {
			return i;
		}
	}
	return 0;
}

uint8_t clientFluidToServer(uint8_t clientFluid)
{
	uint8_t size = sizeof(clientToServerFluidMap) / sizeof(uint8_t);
	if (clientFluid >= size) {
		return 0;
	}
	return clientToServerFluidMap[clientFluid];
}

itemAttrTypes stringToItemAttribute(const std::string& str)
{
	if (str == "aid") {
		return ITEM_ATTRIBUTE_ACTIONID;
	} else if (str == "uid") {
		return ITEM_ATTRIBUTE_UNIQUEID;
	} else if (str == "description") {
		return ITEM_ATTRIBUTE_DESCRIPTION;
	} else if (str == "text") {
		return ITEM_ATTRIBUTE_TEXT;
	} else if (str == "date") {
		return ITEM_ATTRIBUTE_DATE;
	} else if (str == "writer") {
		return ITEM_ATTRIBUTE_WRITER;
	} else if (str == "name") {
		return ITEM_ATTRIBUTE_NAME;
	} else if (str == "article") {
		return ITEM_ATTRIBUTE_ARTICLE;
	} else if (str == "pluralname") {
		return ITEM_ATTRIBUTE_PLURALNAME;
	} else if (str == "weight") {
		return ITEM_ATTRIBUTE_WEIGHT;
	} else if (str == "owner") {
		return ITEM_ATTRIBUTE_OWNER;
	} else if (str == "duration") {
		return ITEM_ATTRIBUTE_DURATION;
	} else if (str == "decaystate") {
		return ITEM_ATTRIBUTE_DECAYSTATE;
	} else if (str == "corpseowner") {
		return ITEM_ATTRIBUTE_CORPSEOWNER;
	} else if (str == "fluidtype") {
		return ITEM_ATTRIBUTE_FLUIDTYPE;
	} else if (str == "doorid") {
		return ITEM_ATTRIBUTE_DOORID;
	} else if (str == "price") {
		return ITEM_ATTRIBUTE_PRICE;
	} else if (str == "pokemoncorpsegender") {
		return ITEM_ATTRIBUTE_POKEMONCORPSEGENDER;
	} else if (str == "pokemoncorpsetype") {
		return ITEM_ATTRIBUTE_POKEMONCORPSETYPE;
	} else if (str == "pokemonid") {
		return ITEM_ATTRIBUTE_POKEMONID;
	} else if (str == "pokemoncorpsenature") {
		return ITEM_ATTRIBUTE_POKEMONCORPSENATURE;
	} else if (str == "pokemoncorpseshinystatus") {
		return ITEM_ATTRIBUTE_POKEMONCORPSESHINYSTATUS;
	}
	return ITEM_ATTRIBUTE_NONE;
}

std::string getFirstLine(const std::string& str)
{
	std::string firstLine;
	firstLine.reserve(str.length());
	for (const char c : str) {
		if (c == '\n') {
			break;
		}
		firstLine.push_back(c);
	}
	return firstLine;
}

const char* getReturnMessage(ReturnValue value)
{
	switch (value) {
		case RETURNVALUE_DESTINATIONOUTOFREACH:
			return "Destination is out of reach.";

		case RETURNVALUE_NOTMOVEABLE:
			return "You cannot move this object.";

		case RETURNVALUE_CANNOTBEDRESSED:
			return "You cannot put this object there.";

		case RETURNVALUE_TOOFARAWAY:
			return "Too far away.";

		case RETURNVALUE_FIRSTGODOWNSTAIRS:
			return "First go downstairs.";

		case RETURNVALUE_FIRSTGOUPSTAIRS:
			return "First go upstairs.";

		case RETURNVALUE_NOTENOUGHCAPACITY:
			return "This object is too heavy for you to carry.";

		case RETURNVALUE_CONTAINERNOTENOUGHROOM:
			return "You cannot put more objects in this container.";

		case RETURNVALUE_NEEDEXCHANGE:
		case RETURNVALUE_NOTENOUGHROOM:
			return "There is not enough room.";

		case RETURNVALUE_CANNOTPICKUP:
			return "You cannot take this object.";

		case RETURNVALUE_CANNOTTHROW:
			return "You cannot throw there.";

		case RETURNVALUE_THEREISNOWAY:
			return "There is no way.";

		case RETURNVALUE_THISISIMPOSSIBLE:
			return "This is impossible.";

		case RETURNVALUE_PLAYERISPZLOCKED:
			return "You can not enter a protection zone after attacking another player.";

		case RETURNVALUE_PLAYERISNOTINVITED:
			return "You are not invited.";

		case RETURNVALUE_CREATUREDOESNOTEXIST:
			return "Creature does not exist.";

		case RETURNVALUE_DEPOTISFULL:
			return "You cannot put more items in this depot.";

		case RETURNVALUE_CANNOTUSETHISOBJECT:
			return "You cannot use this object.";

		case RETURNVALUE_PLAYERWITHTHISNAMEISNOTONLINE:
			return "A player with this name is not online.";

		case RETURNVALUE_YOUAREALREADYTRADING:
			return "You are already trading.";

		case RETURNVALUE_THISPLAYERISALREADYTRADING:
			return "This player is already trading.";

		case RETURNVALUE_YOUMAYNOTLOGOUTDURINGAFIGHT:
			return "You may not logout during or immediately after a fight!";

		case RETURNVALUE_DIRECTPLAYERSHOOT:
			return "You are not allowed to shoot directly on players.";

		case RETURNVALUE_NOTENOUGHLEVEL:
			return "You do not have enough level.";

		case RETURNVALUE_YOUAREEXHAUSTED:
			return "You are exhausted.";

		case RETURNVALUE_PLAYERISNOTREACHABLE:
			return "Player is not reachable.";

		case RETURNVALUE_CREATUREISNOTREACHABLE:
			return "Creature is not reachable.";

		case RETURNVALUE_ACTIONNOTPERMITTEDINPROTECTIONZONE:
			return "This action is not permitted in a protection zone.";

		case RETURNVALUE_YOUMAYNOTATTACKTHISPLAYER:
			return "You may not attack this player.";

		case RETURNVALUE_YOUMAYNOTATTACKTHISCREATURE:
			return "You may not attack this creature.";

		case RETURNVALUE_YOUMAYNOTATTACKAPERSONINPROTECTIONZONE:
			return "You may not attack a person in a protection zone.";

		case RETURNVALUE_YOUMAYNOTATTACKAPERSONWHILEINPROTECTIONZONE:
			return "You may not attack a person while you are in a protection zone.";

		case RETURNVALUE_YOUCANONLYUSEITONCREATURES:
			return "You can only use it on creatures.";

		case RETURNVALUE_YOUNEEDPREMIUMACCOUNT:
			return "You need a premium account.";

		case RETURNVALUE_YOURPROFESSIONCANNOTUSETHISMOVE:
			return "Your profession cannot use this move.";

		case RETURNVALUE_PLAYERISPZLOCKEDLEAVEPVPZONE:
			return "You can not leave a pvp zone after attacking another player.";

		case RETURNVALUE_PLAYERISPZLOCKEDENTERPVPZONE:
			return "You can not enter a pvp zone after attacking another player.";

		case RETURNVALUE_ACTIONNOTPERMITTEDINANOPVPZONE:
			return "This action is not permitted in a non pvp zone.";

		case RETURNVALUE_YOUCANNOTLOGOUTHERE:
			return "You can not logout here.";

		case RETURNVALUE_CANNOTCONJUREITEMHERE:
			return "You cannot conjure items here.";

		case RETURNVALUE_YOUNEEDTOSPLITYOURSPEARS:
			return "You need to split your spears first.";

		case RETURNVALUE_NAMEISTOOAMBIGUOUS:
			return "Player name is ambiguous.";

		case RETURNVALUE_NOPARTYMEMBERSINRANGE:
			return "No party members in range.";

		case RETURNVALUE_YOUARENOTTHEOWNER:
			return "You are not the owner.";

		case RETURNVALUE_NOSUCHRAIDEXISTS:
			return "No such raid exists.";

		case RETURNVALUE_ANOTHERRAIDISALREADYEXECUTING:
			return "Another raid is already executing.";

		case RETURNVALUE_TRADEPLAYERFARAWAY:
			return "Trade player is too far away.";

		case RETURNVALUE_YOUDONTOWNTHISHOUSE:
			return "You don't own this house.";

		case RETURNVALUE_TRADEPLAYERALREADYOWNSAHOUSE:
			return "Trade player already owns a house.";

		case RETURNVALUE_TRADEPLAYERHIGHESTBIDDER:
			return "Trade player is currently the highest bidder of an auctioned house.";

		case RETURNVALUE_YOUCANNOTTRADETHISHOUSE:
			return "You can not trade this house.";

		case RETURNVALUE_CANONLYUSEPOKEBALLINPOKEMON:
			return "You can only use a pokeball in a fainted Pokemon.";

		case RETURNVALUE_CANNOTCAPTURETHISPOKEMON:
			return "You can not capture this Pokemon.";

		case RETURNVALUE_MAXPOKEMONINBAG:
			return "You can only have 6 Pokemon.";

		case RETURNVALUE_CANONLYUSEFOODINYOURPOKEMON:
			return "You can only use a food in your Pokemon.";

		case RETURNVALUE_CANONLYUSEFOODINYOURSELF:
			return "You can only use a food in yourself.";

		case RETURNVALUE_CANONLYUSEFOODINYOURPOKEMONORINYOURSELF:
			return "You can only use a food in your Pokemon or in yourself.";

		case RETURNVALUE_YOURPOKEMONDONOTLIKEIT:
			return "Your Pokemon do not like it.";

		case RETURNVALUE_YOUCANNOTPUTTHISITEM:
			return "You cannot put this item in this container.";

		default: // RETURNVALUE_NOTPOSSIBLE, etc
			return "Sorry, not possible.";
	}
}

const char* getCallPokemonMessage(CallPokemon value)
{
	switch (value) {
		case CALLPOKEMON_MESSAGE_1:
			return ", I need your Help!";

		case CALLPOKEMON_MESSAGE_2:
			return ", it's the battle time.";

		case CALLPOKEMON_MESSAGE_3:
			return ", I choose you!";

		default:
			return ", I choose you!";
	}
}

const char* getCallbackPokemonMessage(CallbackPokemon value)
{
	switch (value) {
		case CALLBACKPOKEMON_MESSAGE_1:
			return ", thanks!";

		case CALLBACKPOKEMON_MESSAGE_2:
			return ", nice work.";

		case CALLBACKPOKEMON_MESSAGE_3:
			return ", that's enough. Come back!";

		default:
			return ", that's enough. Come back!";
	}
}

std::string capitalizeString(std::string str)
{
	str[0] = toupper(str[0]);
	std::transform(str.begin()+1, str.end(),str.begin(),str.begin()+1, 
	[](const char& a, const char& b) -> char
	{
		if(b==' ' || b=='\t')
		{
			return toupper(a);
		}
		return a;
	});

	return str;
}

uint16_t daysUntilTheEnd(time_t tstart, time_t tend)
{
	boost::posix_time::ptime start = boost::posix_time::from_time_t(tstart);
	boost::posix_time::ptime end = boost::posix_time::from_time_t(tend);

	boost::posix_time::time_duration diff = end - start;


	int64_t seconds = diff.total_seconds();

	if (seconds <= 86400) {
		return (seconds / (60 * 60 * 24)) + 1;
	}

	return 0;
}

int64_t OTSYS_TIME()
{
	return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
}