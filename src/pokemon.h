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
class FoodType;

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
		static Pokemon* createPokemon(const std::string& name, int32_t level = 1, bool spawn = true);
		static int32_t despawnRange;
		static int32_t despawnRadius;

		explicit Pokemon(PokemonType* mType, int32_t level = 1, bool spawn = true);
		explicit Pokemon();
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
		const NameColor_t getNameColor() const override {
			return mType->nameColor;
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
			if (getDittoStatus()) {
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
		bool isPushable() const override {
			return mType->info.pushable && getBaseSpeed() != 0;
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
			return ((mType->info.firstType == TYPE_GHOST) || (mType->info.secondType == TYPE_GHOST));
		}

		uint32_t getChargedIcon() const {
			if (isShiny) {
				return mType->info.shiny.iconCharged;
			}
			return mType->info.iconCharged;
		}
		uint32_t getDischargedIcon() const {
			if (isShiny) {
				return mType->info.shiny.iconDischarged;
			}
			return mType->info.iconDischarged;
		}
		uint32_t getLevelToUse() const {
			return mType->info.level;
		}
		uint32_t getPortrait() const {
			if (isShiny) {
				return mType->info.shiny.portrait;
			}
			return mType->info.portrait;
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
		uint32_t getBaseSpeed() const override {
			return (baseSpeed + (ivs.speed + (evs.speed / 30) + mType->info.baseStats.speed) / 4) * getSpeedMultiplier();
		}
		uint16_t getNumber() const {
			return mType->number;
		}
		Natures_t getNature() const {
			return nature;
		}
		bool getShinyStatus() const {
			return isShiny;
		}
		bool getDittoStatus() const {
			return isDitto;
		}
		float getAttack() const override;
		float getDefense() const override;
		float getSpecialAttack() const override;
		float getSpecialDefense() const override;
		virtual int32_t getSpeed() const override {
			return getBaseSpeed() + varSpeed;
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
		void setMaxHealth(int32_t healthMax) {
			this->healthMax = healthMax;
		}
		void setHealth(int32_t health) {
			this->health = std::min<int32_t>(health, getMaxHealth());
		}
		void setPrice(uint32_t price) {
			this->price = price;
		}
		void setShinyStatus(bool isShiny) {
			this->isShiny = isShiny;
		}
		void setDittoStatus(bool isDitto) {
			this->isDitto = isDitto;
		}
		void setPokeballType(const PokeballType* pokeballType) {
			this->pokeballType = pokeballType;
		}
		void setPokemonType(PokemonType* pokemonType);
		float getAttackMultiplier() const;
		float getDefenseMultiplier() const;
		float getSpecialAttackMultiplier() const;
		float getSpecialDefenseMultiplier() const;
		float getSpeedMultiplier() const;
		bool canWalkOnFieldType(CombatType_t combatType) const;


		void onAttackedCreatureDisappear(bool isLogout) override;

		void onCreatureAppear(Creature* creature, bool isLogin) override;
		void onRemoveCreature(Creature* creature, bool isLogout) override;
		void onCreatureMove(Creature* creature, const Tile* newTile, const Position& newPos, const Tile* oldTile, const Position& oldPos, bool teleport) override;
		void onCreatureSay(Creature* creature, SpeakClasses type, const std::string& text) override;

		void drainHealth(Creature* attacker, int32_t damage) override;
		void changeHealth(int32_t healthChange, bool sendHealthChange = true) override;
		void onWalk() override;
		void onWalkComplete() override;
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
			return !isSummon() && getHealth() <= (getHealth() * (mType->info.runAwayHealth / 100.0f));
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

		Player* getTrainer() const {
			if (!getMaster() || !belongsToPlayer()) {
				return nullptr;
			}

			return getMaster()->getPlayer();
		}

		BlockType_t blockHit(Creature* attacker, CombatType_t combatType, int32_t& damage, 
									 bool field = false) override;

		static uint32_t pokemonAutoID;

		void setGUID(uint32_t guid) {
			this->guid = guid;
		}
		uint32_t getGUID() const {
			return guid;
		}

		int32_t getMaxHealth() const override;

		uint32_t getLevel() const {
			return level;
		}
		void setLevel(uint32_t newLevel) {
			if (level == newLevel) {
				return;
			}

			level = newLevel;

			if (getHealth() > getMaxHealth()) {
				setHealth(getMaxHealth());
			}
		}
		uint32_t getXpOnDex() const {
			return mType->info.xpOnDex;
		}

		bool addMove(std::pair<uint16_t, uint16_t> move) {
			if (moves.size() > 12) {
				return false;
			}

			moves[std::get<0>(move)] = std::get<1>(move);
			return true;
		}

		std::map<uint16_t, uint16_t> getMoves() const {
			return moves;
		}

		bool castMove(uint16_t moveId, bool ignoreMessages = false);
		bool feed(const FoodType* foodType) override;
		bool holdingPosition() const {
			return holdPosition;
		}

		void setHoldPosition(bool hold) {
			holdPosition = hold;
		}
		void addSpecialAbility(SpecialAbilities_t specialAbility) {
			abilities |= specialAbility;
		}
		void turnToThing(Thing* thing);

		void setIVHP(uint8_t iv) {
			ivs.hp = iv;
		}
		uint8_t getIVHP() const {
			return ivs.hp;
		}

		void setIVAttack(uint8_t iv) {
			ivs.attack = iv;
		}
		uint8_t getIVAttack() const {
			return ivs.attack;
		}

		void setIVSpecialAttack(uint8_t iv) {
			ivs.special_attack = iv;
		}
		uint8_t getIVSpecialAttack() const {
			return ivs.special_attack;
		}

		void setIVDefense(uint8_t iv) {
			ivs.defense = iv;
		}
		uint8_t getIVDefense() const {
			return ivs.defense;
		}

		void setIVSpecialDefense(uint8_t iv) {
			ivs.special_defense = iv;
		}
		uint8_t getIVSpecialDefense() const {
			return ivs.special_defense;
		}

		uint8_t getIVSpeed() const {
			return ivs.speed;
		}
		void setIVSpeed(uint8_t iv) {
			ivs.speed = iv;
		}

		void setAbilities(uint32_t newAbilities) {
			abilities = newAbilities;
		}
		uint32_t getAbilities() const {
			return abilities;
		}
		bool isSuperEffective(CombatType_t type) const {
			return hasBitSet(static_cast<uint32_t>(type), getDamageSuperEffective());
		}
		bool isNotVeryEffective(CombatType_t type) const {
			return hasBitSet(static_cast<uint32_t>(type), getDamageNotVeryEffective());
		}
		bool hasSpecialAbility(SpecialAbilities_t specialAbility) const {
			return hasBitSet(specialAbility, abilities);
		}
		bool moveTo(const Position& pos, int32_t minTargetDist = 0, int32_t maxTargetDist = 0);
		bool setMaster(Creature* newMaster) override;

		void transform(PokemonType* pmType);		
		void checkCutDigOrRockSmash(Item* item);
		void setNextOrder(std::function<void(void)> order) {
			checkOrder = order;
		}

	private:
		CreatureHashSet friendList;
		CreatureList targetList;
		std::forward_list<Condition*> storedConditionList; // TODO: This variable is only temporarily used when logging in, get rid of it somehow

		std::map<uint16_t, uint16_t> moves;

		uint32_t emotsTicks[3] = {0, 0, 0};

		std::string strDescription;

		PokemonType* mType = nullptr;
		Spawn* spawn = nullptr;
		const PokeballType* pokeballType = nullptr;
		std::function<void(void)> checkOrder = nullptr;

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
		uint32_t level = 1;
		uint32_t abilities = 0;
		int32_t minCombatValue = 0;
		int32_t maxCombatValue = 0;
		int32_t targetChangeCooldown = 0;
		int32_t stepDuration = 0;
		int32_t price = -1;

		Position masterPos;

		bool isDitto = false;
		bool isShiny = false;
		bool isIdle = true;
		bool extraMeleeAttack = false;
		bool isMasterInRange = false;
		bool randomStepping = false;
		bool ignoreFieldDamage = false;
		bool followMaster = true;
		bool holdPosition = false;

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
		void onThinkEmoticon(uint32_t interval);

		bool isFriend(const Creature* creature) const;
		bool isOpponent(const Creature* creature) const;

		uint64_t getLostExperience() const override {
			return skillLoss ? mType->info.experience : 0;
		}
		uint16_t getLookCorpse() const override {
			return mType->info.lookcorpse;
		}
		void dropLoot(Container* corpse, Creature* lastHitCreature) override;
		uint32_t getDamageSuperEffective() const {
			return mType->info.damageSuperEffective;
		}
		uint32_t getDamageNotVeryEffective() const {
			return mType->info.damageNotVeryEffective;
		}
		uint32_t getDamageImmunities() const override {
			return mType->info.damageImmunities;
		}
		uint32_t getConditionImmunities() const override {
			return mType->info.conditionImmunities;
		}
		void getPathSearchParams(const Creature* creature, FindPathParams& fpp) const override;
		bool checkSpawn();
		void basicAttack();
		bool useCacheMap() const override {
			return !randomStepping;
		}

		friend class LuaScriptInterface;
		friend class Game;
		friend class IOLoginData;
};

#endif
