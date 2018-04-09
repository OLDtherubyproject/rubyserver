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

#ifndef FS_POKEMON_H_9F5EEFE64314418CA7DA41D1B9409DD0
#define FS_POKEMON_H_9F5EEFE64314418CA7DA41D1B9409DD0

#include "tile.h"
#include "pokemons.h"

class Creature;
class Game;
class Spawn;
class PokeballType;

using CreatureHashSet = std::unordered_set<Creature*>;
using CreatureList = std::list<Creature*>;

static constexpr int32_t EVENT_SEND_PBEMOT_INTERVAL = 4000;
static constexpr int32_t EVENT_SEND_HOUSEEMOT_INTERVAL = 10000;

enum TargetSearchType_t {
	TARGETSEARCH_DEFAULT,
	TARGETSEARCH_RANDOM,
	TARGETSEARCH_ATTACKRANGE,
	TARGETSEARCH_NEAREST,
};

class Pokemon final : public Creature
{
	public:
		static Pokemon* createPokemon(const std::string& name, bool spawn = true);
		static int32_t despawnRange;
		static int32_t despawnRadius;

		explicit Pokemon(PokemonType* mType, bool spawn = true);
		~Pokemon();

		// non-copyable
		Pokemon(const Pokemon&) = delete;
		Pokemon& operator=(const Pokemon&) = delete;

		Pokemon* getPokemon() override {
			return this;
		}
		const Pokemon* getPokemon() const override {
			return this;
		}

		void setID() override {
			if (id == 0) {
				id = pokemonAutoID++;
			}
		}

		void removeList() override;
		void addList() override;

		const std::string& getName() const override {
			return name;
		}
		const std::string& getNameDescription() const override {
			return mType->nameDescription;
		}
		std::string getDescription(int32_t) const override {
			Creature* creature = getMaster();

			if (creature) {
				return strDescription + ". It belongs to " + creature->getName() + ".";
			}
			return strDescription + '.';			
		}

		CreatureType_t getType() const override {
			return CREATURETYPE_POKEMON;
		}

		const Position& getMasterPos() const {
			return masterPos;
		}
		void setMasterPos(Position pos) {
			masterPos = pos;
		}
		void randomGender() {
			// set pokemon gender
			if (isDitto) {
				return;
			}

			if (mType->info.genders.male > 0 || mType->info.genders.female > 0) {
				if (boolean_random(mType->info.genders.male / 100) == 1) {
					setGender(GENDER_MALE);
				} else{
					setGender(GENDER_FEMALE);
				}
			}
		}

		BloodType_t getBlood() const override {
			return mType->info.blood;
		}
		int32_t getArmor() const override {
			return mType->info.armor;
		}
		int32_t getDefense() const override {
			return mType->info.defense;
		}
		bool isPushable() const override {
			return mType->info.pushable && baseSpeed != 0;
		}
		bool isAttackable() const override {
			return mType->info.isAttackable;
		}

		PokemonType_t getFirstType() const {
			return mType->info.firstType;
		}
		PokemonType_t getSecondType() const {
			return mType->info.secondType;
		}

		bool canEvolve() const {
			return mType->info.canEvolve;
		}
		bool canPushItems() const {
			return mType->info.canPushItems;
		}
		bool canPushCreatures() const {
			return mType->info.canPushCreatures;
		}
		bool isHostile() const {
			return mType->info.isHostile;
		}
		bool isGhost() const {
			return mType->info.isGhost;
		}
		bool canSee(const Position& pos) const override;
		bool canSeeInvisibility() const override {
			return isImmune(CONDITION_INVISIBLE);
		}
		double getCatchRate() const {
			return mType->info.catchRate;
		}
		uint32_t getPrice() const {
			return price;
		}
		uint32_t getBasePrice() const {
			return mType->info.price;
		}
		Natures_t getNature() const {
			return nature;
		}
		uint16_t getHP() const {
			return ivs.hp;
		}
		uint16_t getAtk() const {
			return ivs.attack;
		}
		uint16_t getDef() const {
			return ivs.defense;
		}
		uint16_t getSpeed() const {
			return ivs.speed;
		}
		uint16_t getSpAtk() const {
			return ivs.special_attack;
		}
		uint16_t getSpDef() const {
			return ivs.special_defense;
		}
		std::vector<EvolutionBlock_t> getEvolutions() const {
			return mType->info.evolutions;
		}
		void setSpawn(Spawn* spawn) {
			this->spawn = spawn;
		}
		void setNature(Natures_t nature) {
			this->nature = nature;
		}
		void setName(std::string name) {
			this->name = name;
		}
		void setMaxHealth(int32_t healthMax) {
			this->healthMax = healthMax;
		}
		void setHealth(int32_t health) {
			this->health = std::min<int32_t>(health, getMaxHealth());
		}
		void setPrice(uint32_t price) {
			this->price = price;
		}
		void setIsShiny(bool isShiny) {
			this->isShiny = isShiny;
		}
		void setPokeballType(const PokeballType* pokeballType) {
			this->pokeballType = pokeballType;
		}
		void setPokemonType(PokemonType* pokemonType) {
			this->mType = pokemonType;
		}
		bool canWalkOnFieldType(CombatType_t combatType) const;


		void onAttackedCreatureDisappear(bool isLogout) override;

		void onCreatureAppear(Creature* creature, bool isLogin) override;
		void onRemoveCreature(Creature* creature, bool isLogout) override;
		void onCreatureMove(Creature* creature, const Tile* newTile, const Position& newPos, const Tile* oldTile, const Position& oldPos, bool teleport) override;
		void onCreatureSay(Creature* creature, SpeakClasses type, const std::string& text) override;

		void drainHealth(Creature* attacker, int32_t damage) override;
		void changeHealth(int32_t healthChange, bool sendHealthChange = true) override;
		void onWalk() override;
		bool getNextStep(Direction& direction, uint32_t& flags) override;
		void onFollowCreatureComplete(const Creature* creature) override;

		void onThink(uint32_t interval) override;

		bool challengeCreature(Creature* creature) override;

		void setNormalCreatureLight() override;
		bool getCombatValues(int32_t& min, int32_t& max) override;

		void doAttacking(uint32_t interval) override;
		bool hasExtraSwing() override {
			return extraMeleeAttack;
		}

		bool searchTarget(TargetSearchType_t searchType = TARGETSEARCH_DEFAULT);
		bool selectTarget(Creature* creature);

		const CreatureList& getTargetList() const {
			return targetList;
		}
		const CreatureHashSet& getFriendList() const {
			return friendList;
		}

		bool isTarget(const Creature* creature) const;
		bool isFleeing() const {
			return !isSummon() && getHealth() <= mType->info.runAwayHealth;
		}

		bool getDistanceStep(const Position& targetPos, Direction& direction, bool flee = false);
		bool isTargetNearby() const {
			return stepDuration >= 1;
		}
		bool isIgnoringFieldDamage() const {
			return ignoreFieldDamage;
		}

		const PokeballType* getPokeballType() const {
			return pokeballType;
		}

		PokemonType* getPokemonType() const {
			return mType;
		}
		
		bool teleportToPlayer();

		bool belongsToPlayer() const {
			return isSummon() && getMaster()->getPlayer();
		}
		bool belongsToPlayer(Player* player) const {
			return belongsToPlayer() && (getMaster()->getPlayer() == player);
		}

		BlockType_t blockHit(Creature* attacker, CombatType_t combatType, int32_t& damage,
		                     bool checkDefense = false, bool checkArmor = false, bool field = false) override;

		static uint32_t pokemonAutoID;

		void setGUID(uint32_t guid) {
			this->guid = guid;
		}
		uint32_t getGUID() const {
			return guid;
		}

		int32_t getMaxHealth() const override;

		int32_t getMasterLevel() const {
			return masterLevel;
		}
		void setMasterLevel(int32_t masterLevel) {
			this->masterLevel = masterLevel;
		}

	private:
		CreatureHashSet friendList;
		CreatureList targetList;
		std::forward_list<Condition*> storedConditionList; // TODO: This variable is only temporarily used when logging in, get rid of it somehow

		std::string name;
		std::string strDescription;

		PokemonType* mType;
		Spawn* spawn = nullptr;
		const PokeballType* pokeballType = nullptr;

		Natures_t nature = static_cast<Natures_t>(uniform_random(1, 25));
		PokemonEVs evs = {};
		PokemonIVs ivs = {};

		int64_t lastMeleeAttack = 0;
		int64_t nextEmot = 0;

		uint32_t guid = 0;
		uint32_t attackTicks = 0;
		uint32_t targetTicks = 0;
		uint32_t targetChangeTicks = 0;
		uint32_t defenseTicks = 0;
		uint32_t yellTicks = 0;
		int32_t minCombatValue = 0;
		int32_t maxCombatValue = 0;
		int32_t targetChangeCooldown = 0;
		int32_t stepDuration = 0;
		int32_t price = -1;
		int32_t masterLevel = 1;

		Position masterPos;

		bool isDitto = false;
		bool isShiny = false;
		bool isIdle = true;
		bool extraMeleeAttack = false;
		bool isMasterInRange = false;
		bool randomStepping = false;
		bool ignoreFieldDamage = false;

		void onCreatureEnter(Creature* creature);
		void onCreatureLeave(Creature* creature);
		void onCreatureFound(Creature* creature, bool pushFront = false);

		void updateLookDirection();

		void addFriend(Creature* creature);
		void removeFriend(Creature* creature);
		void addTarget(Creature* creature, bool pushFront = false);
		void removeTarget(Creature* creature);

		void updateTargetList();
		void clearTargetList();
		void clearFriendList();

		void death(Creature* lastHitCreature) override;
		Item* getCorpse(Creature* lastHitCreature, Creature* mostDamageCreature) override;

		void setIdle(bool idle);
		void updateIdleStatus();
		bool getIdleStatus() const {
			return isIdle;
		}

		//combat event functions
		void onAddCondition(ConditionType_t type) override;
		void onAddCombatCondition(ConditionType_t type) override;
		void onEndCondition(ConditionType_t type) override;
		void onCombatRemoveCondition(Condition* condition) override;

		bool canUseAttack(const Position& pos, const Creature* target) const;
		bool canUseMove(const Position& pos, const Position& targetPos,
		                 const moveBlock_t& sb, uint32_t interval, bool& inRange, bool& resetTicks);
		bool getRandomStep(const Position& creaturePos, Direction& direction) const;
		bool getDanceStep(const Position& creaturePos, Direction& direction,
		                  bool keepAttack = true, bool keepDistance = true);
		bool isInSpawnRange(const Position& pos) const;
		bool canWalkTo(Position pos, Direction direction) const;

		bool canSendEmot() const {
			return nextEmot < OTSYS_TIME();
		}
		void setNextEmot(int64_t time) {
			if (time > nextEmot) {
				nextEmot = time;
			}
		}

		static bool pushItem(Item* item);
		static void pushItems(Tile* tile);
		static bool pushCreature(Creature* creature);
		static void pushCreatures(Tile* tile);

		void onThinkTarget(uint32_t interval);
		void onThinkYell(uint32_t interval);
		void onThinkDefense(uint32_t interval);

		bool isFriend(const Creature* creature) const;
		bool isOpponent(const Creature* creature) const;

		uint64_t getLostExperience() const override {
			return skillLoss ? mType->info.experience : 0;
		}
		uint16_t getLookCorpse() const override {
			return mType->info.lookcorpse;
		}
		void dropLoot(Container* corpse, Creature* lastHitCreature) override;
		uint32_t getDamageImmunities() const override {
			return mType->info.damageImmunities;
		}
		uint32_t getConditionImmunities() const override {
			return mType->info.conditionImmunities;
		}
		void getPathSearchParams(const Creature* creature, FindPathParams& fpp) const override;
		bool useCacheMap() const override {
			return !randomStepping;
		}

		friend class LuaScriptInterface;
		friend class Game;
};

#endif
