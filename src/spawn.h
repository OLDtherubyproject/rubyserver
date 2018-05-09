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

#ifndef FS_SPAWN_H_1A86089E080846A9AE53ED12E7AE863B
#define FS_SPAWN_H_1A86089E080846A9AE53ED12E7AE863B

#include "tile.h"
#include "position.h"

class Pokemon;
class PokemonType;
class Npc;

struct spawnBlock_t {
	Position pos;
	PokemonType* mType;
	int64_t lastSpawn;
	uint32_t interval;
	Direction direction;
};

class Spawn
{
	public:
		Spawn(Position pos, int32_t radius, int32_t minlevel, int32_t maxlevel) : centerPos(std::move(pos)), radius(radius), minlevel(minlevel), maxlevel(maxlevel) {}
		~Spawn();

		// non-copyable
		Spawn(const Spawn&) = delete;
		Spawn& operator=(const Spawn&) = delete;

		bool addPokemon(const std::string& name, const Position& pos, Direction dir, uint32_t interval);
		void removePokemon(Pokemon* pokemon);

		uint32_t getInterval() const {
			return interval;
		}
		void startup();

		void startSpawnCheck();
		void stopEvent();

		bool isInSpawnZone(const Position& pos);
		void cleanup();

	private:
		//map of the spawned creatures
		using SpawnedMap = std::multimap<uint32_t, Pokemon*>;
		using spawned_pair = SpawnedMap::value_type;
		SpawnedMap spawnedMap;

		//map of creatures in the spawn
		std::map<uint32_t, spawnBlock_t> spawnMap;

		Position centerPos;
		int32_t radius;
		int32_t minlevel;
		int32_t maxlevel;

		uint32_t interval = 60000;
		uint32_t checkSpawnEvent = 0;

		static bool findPlayer(const Position& pos);
		bool spawnPokemon(uint32_t spawnId, PokemonType* mType, const Position& pos, Direction dir, bool startup = false);
		void checkSpawn();
};

class Spawns
{
	public:
		static bool isInZone(const Position& centerPos, int32_t radius, const Position& pos);

		bool loadFromXml(const std::string& filename);
		void startup();
		void clear();

		bool isStarted() const {
			return started;
		}

	private:
		std::forward_list<Npc*> npcList;
		std::forward_list<Spawn> spawnList;
		std::string filename;
		bool loaded = false;
		bool started = false;
};

#endif
