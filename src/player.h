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

#ifndef FS_PLAYER_H_4083D3D3A05B4EDE891B31BB720CD06F
#define FS_PLAYER_H_4083D3D3A05B4EDE891B31BB720CD06F

#include "creature.h"
#include "pokemon.h"
#include "container.h"
#include "cylinder.h"
#include "outfit.h"
#include "enums.h"
#include "profession.h"
#include "clan.h"
#include "protocolgame.h"
#include "ioguild.h"
#include "party.h"
#include "inbox.h"
#include "depotchest.h"
#include "depotlocker.h"
#include "guild.h"
#include "groups.h"
#include "town.h"
#include "mounts.h"

class House;
class NetworkMessage;
class ProtocolGame;
class Npc;
class Party;
class SchedulerTask;
class Bed;
class Guild;
class FoodType;

enum skillsid_t {
	SKILLVALUE_LEVEL = 0,
	SKILLVALUE_TRIES = 1,
	SKILLVALUE_PERCENT = 2,
};

enum fightMode_t : uint8_t {
	FIGHTMODE_ATTACK = 1,
	FIGHTMODE_BALANCED = 2,
	FIGHTMODE_DEFENSE = 3,
};

enum pvpMode_t : uint8_t {
	PVP_MODE_DOVE = 0,
	PVP_MODE_WHITE_HAND = 1,
	PVP_MODE_YELLOW_HAND = 2,
	PVP_MODE_RED_FIST = 3,
};

enum tradestate_t : uint8_t {
	TRADE_NONE,
	TRADE_INITIATED,
	TRADE_ACCEPT,
	TRADE_ACKNOWLEDGE,
	TRADE_TRANSFER,
};

struct VIPEntry {
	VIPEntry(uint32_t guid, std::string name, std::string description, uint32_t icon, bool notify) :
		guid(guid), name(std::move(name)), description(std::move(description)), icon(icon), notify(notify) {}

	uint32_t guid;
	std::string name;
	std::string description;
	uint32_t icon;
	bool notify;
};

struct OpenContainer {
	Container* container;
	uint16_t index;
};

struct OutfitEntry {
	constexpr OutfitEntry(uint16_t lookType, uint8_t addons) : lookType(lookType), addons(addons) {}

	uint16_t lookType;
	uint8_t addons;
};

struct Skill {
	uint64_t tries = 0;
	uint16_t level = 10;
	uint8_t percent = 0;
};

using MuteCountMap = std::map<uint32_t, uint32_t>;

static constexpr int32_t PLAYER_MAX_SPEED = 1500;
static constexpr int32_t PLAYER_MIN_SPEED = 10;

class Player final : public Creature, public Cylinder
{
	public:
		explicit Player(ProtocolGame_ptr p);
		~Player();

		// non-copyable
		Player(const Player&) = delete;
		Player& operator=(const Player&) = delete;

		Player* getPlayer() override {
			return this;
		}
		const Player* getPlayer() const override {
			return this;
		}

		void setID() override {
			if (id == 0) {
				id = playerAutoID++;
			}
		}

		static MuteCountMap muteCountMap;

		const std::string& getName() const override {
			return name;
		}
		void setName(std::string name) {
			this->name = std::move(name);
		}
		const std::string& getNameDescription() const override {
			return name;
		}
		std::string getDescription(int32_t lookDistance) const override;
		const NameColor_t getNameColor() const override {
			return getGroup()->nameColor;
		}

		CreatureType_t getType() const override {
			return CREATURETYPE_PLAYER;
		}

		uint8_t getCurrentMount() const;
		void setCurrentMount(uint8_t mountId);
		bool isMounted() const {
			return defaultOutfit.lookMount != 0;
		}
		bool toggleMount(bool mount);
		bool tameMount(uint8_t mountId);
		bool untameMount(uint8_t mountId);
		bool hasMount(const Mount* mount) const;
		void dismount();

		void sendFYIBox(const std::string& message) {
			if (client) {
				client->sendFYIBox(message);
			}
		}

		void setGUID(uint32_t guid) {
			this->guid = guid;
		}
		uint32_t getGUID() const {
			return guid;
		}
		bool canSeeInvisibility() const override {
			return hasFlag(PlayerFlag_CanSenseInvisibility) || group->access;
		}

		void removeList() override;
		void addList() override;
		void kickPlayer(bool displayEffect);

		static uint64_t getExpForLevel(int32_t lv) {
			lv--;
			return ((50ULL * lv * lv * lv) - (150ULL * lv * lv) + (400ULL * lv)) / 3ULL;
		}

		uint16_t getStaminaMinutes() const {
			return staminaMinutes;
		}

		uint64_t getBankBalance() const {
			return bankBalance;
		}
		void setBankBalance(uint64_t balance) {
			bankBalance = balance;
		}

		Guild* getGuild() const {
			return guild;
		}
		void setGuild(Guild* guild);

		const GuildRank* getGuildRank() const {
			return guildRank;
		}
		void setGuildRank(const GuildRank* newGuildRank) {
			guildRank = newGuildRank;
		}

		bool isGuildMate(const Player* player) const;

		const std::string& getGuildNick() const {
			return guildNick;
		}
		void setGuildNick(std::string nick) {
			guildNick = nick;
		}

		bool isInWar(const Player* player) const;
		bool isInWarList(uint32_t guildId) const;

		void setLastWalkthroughAttempt(int64_t walkthroughAttempt) {
			lastWalkthroughAttempt = walkthroughAttempt;
		}
		void setLastWalkthroughPosition(Position walkthroughPosition) {
			lastWalkthroughPosition = walkthroughPosition;
		}

		Inbox* getInbox() const {
			return inbox;
		}

		uint16_t getClientIcons() const;

		const GuildWarVector& getGuildWarVector() const {
			return guildWarVector;
		}

		Profession* getProfession() const {
			return profession;
		}

		Clan* getClan() const {
			return clan;
		}

		OperatingSystem_t getOperatingSystem() const {
			return operatingSystem;
		}
		void setOperatingSystem(OperatingSystem_t clientos) {
			operatingSystem = clientos;
		}

		uint16_t getProtocolVersion() const {
			if (!client) {
				return 0;
			}

			return client->getVersion();
		}

		void setParty(Party* party) {
			this->party = party;
		}
		Party* getParty() const {
			return party;
		}
		PartyShields_t getPartyShield(const Player* player) const;
		bool isInviting(const Player* player) const;
		bool isPartner(const Player* player) const;
		void sendPlayerPartyIcons(Player* player);
		bool addPartyInvitation(Party* party);
		void removePartyInvitation(Party* party);
		void clearPartyInvitations();

		GuildEmblems_t getGuildEmblem(const Player* player) const;

		bool hasFlag(PlayerFlags value) const {
			return (group->flags & value) != 0;
		}

		void addBlessing(uint8_t blessing) {
			blessings |= blessing;
		}
		void removeBlessing(uint8_t blessing) {
			blessings &= ~blessing;
		}
		bool hasBlessing(uint8_t value) const {
			return (blessings & (static_cast<uint8_t>(1) << value)) != 0;
		}

		bool isOffline() const {
			return (getID() == 0);
		}
		void disconnect() {
			if (client) {
				client->disconnect();
			}
		}
		uint32_t getIP() const;

		void addContainer(uint8_t cid, Container* container);
		void closeContainer(uint8_t cid);
		void setContainerIndex(uint8_t cid, uint16_t index);

		Container* getContainerByID(uint8_t cid);
		int8_t getContainerID(const Container* container) const;
		uint16_t getContainerIndex(uint8_t cid) const;

		bool canOpenCorpse(uint32_t ownerId) const;

		void addStorageValue(const uint32_t key, const int32_t value, const bool isLogin = false);
		bool getStorageValue(const uint32_t key, int32_t& value) const;
		void genReservedStorageRange();

		void setGroup(Group* newGroup) {
			group = newGroup;
		}
		Group* getGroup() const {
			return group;
		}

		void setInMarket(bool value) {
			inMarket = value;
		}
		bool isInMarket() const {
			return inMarket;
		}

		void setPokemonHealthMax(uint32_t health) {
			pokemonHealthMax = health;
		}
		void setPokemonHealth(uint32_t health) {
			pokemonHealth = health;
		}

		void setLastDepotId(int16_t newId) {
			lastDepotId = newId;
		}
		int16_t getLastDepotId() const {
			return lastDepotId;
		}

		void resetIdleTime() {
			idleTime = 0;
		}

		bool isInGhostMode() const override {
			return ghostMode;
		}
		void switchGhostMode() {
			ghostMode = !ghostMode;
		}

		uint32_t getAccount() const {
			return accountNumber;
		}
		AccountType_t getAccountType() const {
			return accountType;
		}
		uint32_t getLevel() const {
			return level;
		}
		uint8_t getLevelPercent() const {
			return levelPercent;
		}
		uint8_t getPokemonCapacity() const {
			return pokemonCapacity;
		}
		bool isAccessPlayer() const {
			return group->access;
		}
		bool isPremium() const;
		void setPremiumDays(int32_t v);

		uint16_t getHelpers() const;

		bool setProfession(uint16_t profId);
		uint16_t getProfessionId() const {
			return profession->getId();
		}

		bool setClan(uint16_t clanId);
		uint16_t getClanId() const {
			return clan->getId();
		}

		PlayerSex_t getSex() const {
			return sex;
		}
		void setSex(PlayerSex_t);
		uint64_t getExperience() const {
			return experience;
		}

		time_t getLastLoginSaved() const {
			return lastLoginSaved;
		}

		time_t getLastLogout() const {
			return lastLogout;
		}

		const Position& getLoginPosition() const {
			return loginPosition;
		}
		const Position& getTemplePosition() const {
			return town->getTemplePosition();
		}
		Town* getTown() const {
			return town;
		}
		void setTown(Town* town) {
			this->town = town;
		}

		void clearModalWindows();
		bool hasModalWindowOpen(uint32_t modalWindowId) const;
		void onModalWindowHandled(uint32_t modalWindowId);

		bool isPushable() const override;
		uint32_t isMuted() const;
		void addMessageBuffer();
		void removeMessageBuffer();

		bool removeItemOfType(uint16_t itemId, uint32_t amount, int32_t subType, bool ignoreEquipped = false) const;

		uint32_t getCapacity() const {
			if (hasFlag(PlayerFlag_CannotPickupItem)) {
				return 0;
			} else if (hasFlag(PlayerFlag_HasInfiniteCapacity)) {
				return std::numeric_limits<uint32_t>::max();
			}
			return capacity;
		}

		uint32_t getFreeCapacity() const {
			if (hasFlag(PlayerFlag_CannotPickupItem)) {
				return 0;
			} else if (hasFlag(PlayerFlag_HasInfiniteCapacity)) {
				return std::numeric_limits<uint32_t>::max();
			} else {
				return std::max<int32_t>(0, capacity - inventoryWeight);
			}
		}

		int32_t getMaxHealth() const override {
			return std::max<int32_t>(1, healthMax + varStats[STAT_MAXHITPOINTS]);
		}
		uint32_t getPokemonHealth() const {
			return pokemonHealth;
		}
		uint32_t getPokemonHealthMax() const {
			return pokemonHealthMax;
		}

		Item* getInventoryItem(slots_t slot) const;

		bool isItemAbilityEnabled(slots_t slot) const {
			return inventoryAbilities[slot];
		}
		void setItemAbility(slots_t slot, bool enabled) {
			inventoryAbilities[slot] = enabled;
		}

		void setVarSkill(skills_t skill, int32_t modifier) {
			varSkills[skill] += modifier;
		}

		void setVarSpecialSkill(SpecialSkills_t skill, int32_t modifier) {
			varSpecialSkills[skill] += modifier;
		}

		void setVarStats(stats_t stat, int32_t modifier);
		int32_t getDefaultStats(stats_t stat) const;

		void addConditionSuppressions(uint32_t conditions);
		void removeConditionSuppressions(uint32_t conditions);

		DepotChest* getDepotChest(uint32_t depotId, bool autoCreate);
		DepotLocker* getDepotLocker(uint32_t depotId);
		void onReceiveMail() const;
		bool isNearDepotBox() const;

		bool canSee(const Position& pos) const override;
		bool canSeeCreature(const Creature* creature) const override;

		bool canWalkthrough(const Creature* creature) const;
		bool canWalkthroughEx(const Creature* creature) const;

		BloodType_t getBlood() const override {
			return BLOOD_RED;
		}

		uint64_t getMoney() const;

		//safe-trade functions
		void setTradeState(tradestate_t state) {
			tradeState = state;
		}
		tradestate_t getTradeState() const {
			return tradeState;
		}
		Item* getTradeItem() {
			return tradeItem;
		}

		//shop functions
		void setShopOwner(Npc* owner, int32_t onBuy, int32_t onSell) {
			shopOwner = owner;
			purchaseCallback = onBuy;
			saleCallback = onSell;
		}

		Npc* getShopOwner(int32_t& onBuy, int32_t& onSell) {
			onBuy = purchaseCallback;
			onSell = saleCallback;
			return shopOwner;
		}

		const Npc* getShopOwner(int32_t& onBuy, int32_t& onSell) const {
			onBuy = purchaseCallback;
			onSell = saleCallback;
			return shopOwner;
		}

		//V.I.P. functions
		void notifyStatusChange(Player* loginPlayer, VipStatus_t status);
		bool removeVIP(uint32_t vipGuid);
		bool addVIP(uint32_t vipGuid, const std::string& vipName, VipStatus_t status);
		bool addVIPInternal(uint32_t vipGuid);
		bool editVIP(uint32_t vipGuid, const std::string& description, uint32_t icon, bool notify);

		//follow functions
		bool setFollowCreature(Creature* creature) override;
		void goToFollowCreature() override;

		//follow events
		void onFollowCreature(const Creature* creature) override;

		//walk events
		void onWalk(Direction& dir) override;
		void onWalkAborted() override;
		void onWalkComplete() override;

		void stopWalk();
		void openShopWindow(Npc* npc, const std::list<ShopInfo>& shop);
		bool closeShopWindow(bool sendCloseShopWindow = true);
		bool updateSaleShopList(const Item* item);
		bool hasShopItemForSale(uint32_t itemId, uint16_t subType) const;

		void setChaseMode(bool mode);
		void setFightMode(fightMode_t mode) {
			fightMode = mode;
		}

		//combat functions
		bool setAttackedCreature(Creature* creature) override;
		bool isImmune(CombatType_t type) const override;
		bool isImmune(ConditionType_t type) const override;
		bool isAttackable() const override;
		static bool lastHitIsPlayer(Creature* lastHitCreature);

		void changeHealth(int32_t healthChange, bool sendHealthChange = true) override;
		void changePokemonHealth(int32_t hpChange);
		void changePokemonCapacity(int32_t pkCapacityChange);

		bool isPzLocked() const {
			return pzLocked;
		}
		BlockType_t blockHit(Creature* attacker, CombatType_t combatType, int32_t& damage,
		                             bool field = false) override;
		void doAttacking(uint32_t interval) override;

		uint16_t getSpecialSkill(uint8_t skill) const {
			return std::max<int32_t>(0, varSpecialSkills[skill]);
		}
		uint16_t getSkillLevel(uint8_t skill) const {
			return std::max<int32_t>(0, skills[skill].level + varSkills[skill]);
		}
		uint16_t getBaseSkill(uint8_t skill) const {
			return skills[skill].level;
		}
		uint8_t getSkillPercent(uint8_t skill) const {
			return skills[skill].percent;
		}

		bool getAddAttackSkill() const {
			return addAttackSkillPoint;
		}

		void drainHealth(Creature* attacker, int32_t damage) override;
		void addSkillAdvance(skills_t skill, uint64_t count);

		float getDefense() const override;
		float getSpecialDefense() const override;
		float getAttackFactor() const override;
		float getDefenseFactor() const override;

		void addInFightTicks(bool pzlock = false);

		uint64_t getGainedExperience(Creature* attacker) const override;

		//combat event functions
		void onAddCondition(ConditionType_t type) override;
		void onAddCombatCondition(ConditionType_t type) override;
		void onEndCondition(ConditionType_t type) override;
		void onCombatRemoveCondition(Condition* condition) override;
		void onAttackedCreature(Creature* target) override;
		void onAttacked() override;
		void onAttackedCreatureDrainHealth(Creature* target, int32_t points) override;
		void onTargetCreatureGainHealth(Creature* target, int32_t points) override;
		bool onKilledCreature(Creature* target, bool lastHit = true) override;
		void onGainExperience(uint64_t gainExp, Creature* target) override;
		void onGainSharedExperience(uint64_t gainExp, Creature* source);
		void onChangeZone(ZoneType_t zone) override;
		void onAttackedCreatureChangeZone(ZoneType_t zone) override;
		void onIdleStatus() override;
		void onPlacedCreature() override;

		LightInfo getCreatureLight() const override;

		Genders_t getGender() const override;
		Genders_t getGenderClient(const Creature* creature) const override;

		bool hasAttacked(const Player* attacked) const;
		void addAttacked(const Player* attacked);
		void removeAttacked(const Player* attacked);
		void clearAttacked();
		void addUnjustifiedDead(const Player* attacked);
		void sendCreatureGender(const Creature* creature) const {
			if (client) {
				client->sendCreatureGender(creature);
			}
		}

		bool canWear(uint32_t lookType, uint8_t addons) const;
		void addOutfit(uint16_t lookType, uint8_t addons);
		bool removeOutfit(uint16_t lookType);
		bool removeOutfitAddon(uint16_t lookType, uint8_t addons);
		bool getOutfitAddons(const Outfit& outfit, uint8_t& addons) const;

		bool canLogout();

		size_t getMaxVIPEntries() const;
		size_t getMaxDepotItems() const;

		//tile
		//send methods
		void sendAnimatedText(const Position& pos, uint8_t color, std::string text) const {
       		if (client) client->sendAnimatedText(pos, color, text);
    	}
		void sendAddTileItem(const Tile* tile, const Position& pos, const Item* item) {
			if (client) {
				int32_t stackpos = tile->getStackposOfItem(this, item);
				if (stackpos != -1) {
					client->sendAddTileItem(pos, stackpos, item);
				}
			}
		}
		void sendUpdateTileItem(const Tile* tile, const Position& pos, const Item* item) {
			if (client) {
				int32_t stackpos = tile->getStackposOfItem(this, item);
				if (stackpos != -1) {
					client->sendUpdateTileItem(pos, stackpos, item);
				}
			}
		}
		void sendRemoveTileThing(const Position& pos, int32_t stackpos) {
			if (stackpos != -1 && client) {
				client->sendRemoveTileThing(pos, stackpos);
			}
		}
		void sendUpdateTile(const Tile* tile, const Position& pos) {
			if (client) {
				client->sendUpdateTile(tile, pos);
			}
		}

		void sendChannelMessage(const std::string& author, const std::string& text, SpeakClasses type, uint16_t channel) {
			if (client) {
				client->sendChannelMessage(author, text, type, channel);
			}
		}
		void sendChannelEvent(uint16_t channelId, const std::string& playerName, ChannelEvent_t channelEvent) {
			if (client) {
				client->sendChannelEvent(channelId, playerName, channelEvent);
			}
		}
		void sendCreatureAppear(const Creature* creature, const Position& pos, bool isLogin) {
			if (client) {
				client->sendAddCreature(creature, pos, creature->getTile()->getStackposOfCreature(this, creature), isLogin);
			}
		}
		void sendCreatureMove(const Creature* creature, const Position& newPos, int32_t newStackPos, const Position& oldPos, int32_t oldStackPos, bool teleport) {
			if (client) {
				client->sendMoveCreature(creature, newPos, newStackPos, oldPos, oldStackPos, teleport);
			}
		}
		void sendCreatureTurn(const Creature* creature) {
			if (client && canSeeCreature(creature)) {
				int32_t stackpos = creature->getTile()->getStackposOfCreature(this, creature);
				if (stackpos != -1) {
					client->sendCreatureTurn(creature, stackpos);
				}
			}
		}
		void sendCreatureSay(const Creature* creature, SpeakClasses type, const std::string& text, const Position* pos = nullptr) {
			if (client) {
				client->sendCreatureSay(creature, type, text, pos);
			}
		}
		void sendPrivateMessage(const Player* speaker, SpeakClasses type, const std::string& text) {
			if (client) {
				client->sendPrivateMessage(speaker, type, text);
			}
		}
		void sendCreatureSquare(const Creature* creature, SquareColor_t color) {
			if (client) {
				client->sendCreatureSquare(creature, color);
			}
		}
		void sendCreatureChangeOutfit(const Creature* creature, const Outfit_t& outfit) {
			if (client) {
				client->sendCreatureOutfit(creature, outfit);
			}
		}
		void sendCreatureChangeVisible(const Creature* creature, bool visible) {
			if (!client) {
				return;
			}

			if (creature->getPlayer()) {
				if (visible) {
					client->sendCreatureOutfit(creature, creature->getCurrentOutfit());
				} else {
					static Outfit_t outfit;
					client->sendCreatureOutfit(creature, outfit);
				}
			} else if (canSeeInvisibility()) {
				client->sendCreatureOutfit(creature, creature->getCurrentOutfit());
			} else {
				int32_t stackpos = creature->getTile()->getStackposOfCreature(this, creature);
				if (stackpos == -1) {
					return;
				}

				if (visible) {
					client->sendAddCreature(creature, creature->getPosition(), stackpos, false);
				} else {
					client->sendRemoveTileThing(creature->getPosition(), stackpos);
				}
			}
		}
		void sendCreatureLight(const Creature* creature) {
			if (client) {
				client->sendCreatureLight(creature);
			}
		}
		void sendCreatureWalkthrough(const Creature* creature, bool walkthrough) {
			if (client) {
				client->sendCreatureWalkthrough(creature, walkthrough);
			}
		}
		void sendCreatureShield(const Creature* creature) {
			if (client) {
				client->sendCreatureShield(creature);
			}
		}
		void sendCreatureType(uint32_t creatureId, uint8_t creatureType) {
			if (client) {
				client->sendCreatureType(creatureId, creatureType);
			}
		}
		void sendCreatureHelpers(uint32_t creatureId, uint16_t helpers) {
			if (client) {
				client->sendCreatureHelpers(creatureId, helpers);
			}
		}
		void sendPokemonMoveCooldown(uint16_t moveId, uint32_t time) {
			if (client) {
				client->sendPokemonMoveCooldown(moveId, time);
			}
		}
		void sendPokemonMoves(Pokemon* pokemon) {
			if (client) {
				client->sendPokemonMoves(pokemon);
			}
		}
		void sendModalWindow(const ModalWindow& modalWindow);

		//container
		void sendAddContainerItem(const Container* container, const Item* item);
		void sendUpdateContainerItem(const Container* container, uint16_t slot, const Item* newItem);
		void sendRemoveContainerItem(const Container* container, uint16_t slot);
		void sendContainer(uint8_t cid, const Container* container, bool hasParent, uint16_t firstIndex) {
			if (client) {
				client->sendContainer(cid, container, hasParent, firstIndex);
			}
		}

		//inventory
		void sendInventoryItem(slots_t slot, const Item* item) {
			if (client) {
				client->sendInventoryItem(slot, item);
			}
		}
		void sendItems() {
			if (client) {
				client->sendItems();
			}
		}

		//event methods
		void onUpdateTileItem(const Tile* tile, const Position& pos, const Item* oldItem,
		                              const ItemType& oldType, const Item* newItem, const ItemType& newType) override;
		void onRemoveTileItem(const Tile* tile, const Position& pos, const ItemType& iType,
		                              const Item* item) override;

		void onCreatureAppear(Creature* creature, bool isLogin) override;
		void onRemoveCreature(Creature* creature, bool isLogout) override;
		void onCreatureMove(Creature* creature, const Tile* newTile, const Position& newPos,
		                            const Tile* oldTile, const Position& oldPos, bool teleport) override;

		void onAttackedCreatureDisappear(bool isLogout) override;
		void onFollowCreatureDisappear(bool isLogout) override;

		//container
		void onAddContainerItem(const Item* item);
		void onUpdateContainerItem(const Container* container, const Item* oldItem, const Item* newItem);
		void onRemoveContainerItem(const Container* container, const Item* item);

		void onCloseContainer(const Container* container);
		void onSendContainer(const Container* container);
		void autoCloseContainers(const Container* container);

		//inventory
		void onUpdateInventoryItem(Item* oldItem, Item* newItem);
		void onRemoveInventoryItem(Item* item);

		void sendCancelMessage(const std::string& msg) const {
			if (client) {
				client->sendTextMessage(TextMessage(MESSAGE_STATUS_SMALL, msg));
			}
		}
		void sendCancelMessage(ReturnValue message) const;
		void sendCancelTarget() const {
			if (client) {
				client->sendCancelTarget();
			}
		}
		void sendCancelWalk() const {
			if (client) {
				client->sendCancelWalk();
			}
		}
		void sendChangeSpeed(const Creature* creature, uint32_t newSpeed) const {
			if (client) {
				client->sendChangeSpeed(creature, newSpeed);
			}
		}
		void sendCreatureHealth(const Creature* creature) const {
			if (client) {
				client->sendCreatureHealth(creature);
			}
		}
		void sendDistanceShoot(const Position& from, const Position& to, uint16_t type) const {
			if (client) {
				client->sendDistanceShoot(from, to, type);
			}
		}
		void sendHouseWindow(House* house, uint32_t listId) const;
		void sendCreatePrivateChannel(uint16_t channelId, const std::string& channelName) {
			if (client) {
				client->sendCreatePrivateChannel(channelId, channelName);
			}
		}
		void sendClosePrivate(uint16_t channelId);
		void sendIcons() const {
			if (client) {
				client->sendIcons(getClientIcons());
			}
		}
		void sendEffect(const Position& pos, uint16_t type) const {
			if (client) {
				client->sendEffect(pos, type);
			}
		}
		void sendPing();
		void sendPingBack() const {
			if (client) {
				client->sendPingBack();
			}
		}
		void sendStats();
		void sendBasicData() const {
			if (client) {
				client->sendBasicData();
			}
		}
		void sendSkills() const {
			if (client) {
				client->sendSkills();
			}
		}
		void sendTextMessage(MessageClasses mclass, const std::string& message) const {
			if (client) {
				client->sendTextMessage(TextMessage(mclass, message));
			}
		}
		void sendTextMessage(const TextMessage& message) const {
			if (client) {
				client->sendTextMessage(message);
			}
		}
		void sendReLoginWindow(uint8_t unfairFightReduction) const {
			if (client) {
				client->sendReLoginWindow(unfairFightReduction);
			}
		}
		void sendTextWindow(Item* item, uint16_t maxlen, bool canWrite) const {
			if (client) {
				client->sendTextWindow(windowTextId, item, maxlen, canWrite);
			}
		}
		void sendTextWindow(uint32_t itemId, const std::string& text) const {
			if (client) {
				client->sendTextWindow(windowTextId, itemId, text);
			}
		}
		void sendToChannel(const Creature* creature, SpeakClasses type, const std::string& text, uint16_t channelId) const {
			if (client) {
				client->sendToChannel(creature, type, text, channelId);
			}
		}
		void sendShop(Npc* npc) const {
			if (client) {
				client->sendShop(npc, shopItemList);
			}
		}
		void sendSaleItemList() const {
			if (client) {
				client->sendSaleItemList(shopItemList);
			}
		}
		void sendCloseShop() const {
			if (client) {
				client->sendCloseShop();
			}
		}
		void sendMarketEnter(uint32_t depotId) const {
			if (client) {
				client->sendMarketEnter(depotId);
			}
		}
		void sendMarketLeave() {
			inMarket = false;
			if (client) {
				client->sendMarketLeave();
			}
		}
		void sendMarketBrowseItem(uint16_t itemId, const MarketOfferList& buyOffers, const MarketOfferList& sellOffers) const {
			if (client) {
				client->sendMarketBrowseItem(itemId, buyOffers, sellOffers);
			}
		}
		void sendMarketBrowseOwnOffers(const MarketOfferList& buyOffers, const MarketOfferList& sellOffers) const {
			if (client) {
				client->sendMarketBrowseOwnOffers(buyOffers, sellOffers);
			}
		}
		void sendMarketBrowseOwnHistory(const HistoryMarketOfferList& buyOffers, const HistoryMarketOfferList& sellOffers) const {
			if (client) {
				client->sendMarketBrowseOwnHistory(buyOffers, sellOffers);
			}
		}
		void sendMarketDetail(uint16_t itemId) const {
			if (client) {
				client->sendMarketDetail(itemId);
			}
		}
		void sendMarketAcceptOffer(const MarketOfferEx& offer) const {
			if (client) {
				client->sendMarketAcceptOffer(offer);
			}
		}
		void sendMarketCancelOffer(const MarketOfferEx& offer) const {
			if (client) {
				client->sendMarketCancelOffer(offer);
			}
		}
		void sendTradeItemRequest(const std::string& traderName, const Item* item, bool ack) const {
			if (client) {
				client->sendTradeItemRequest(traderName, item, ack);
			}
		}
		void sendTradeClose() const {
			if (client) {
				client->sendCloseTrade();
			}
		}
		void sendWorldLight(LightInfo lightInfo) {
			if (client) {
				client->sendWorldLight(lightInfo);
			}
		}
		void sendChannelsDialog() {
			if (client) {
				client->sendChannelsDialog();
			}
		}
		void sendOpenPrivateChannel(const std::string& receiver) {
			if (client) {
				client->sendOpenPrivateChannel(receiver);
			}
		}
		void sendOutfitWindow() {
			if (client) {
				client->sendOutfitWindow();
			}
		}
		void sendCloseContainer(uint8_t cid) {
			if (client) {
				client->sendCloseContainer(cid);
			}
		}

		void sendChannel(uint16_t channelId, const std::string& channelName, const UsersMap* channelUsers, const InvitedMap* invitedUsers) {
			if (client) {
				client->sendChannel(channelId, channelName, channelUsers, invitedUsers);
			}
		}
		void sendTutorial(uint8_t tutorialId) {
			if (client) {
				client->sendTutorial(tutorialId);
			}
		}
		void sendAddMarker(const Position& pos, uint8_t markType, const std::string& desc) {
			if (client) {
				client->sendAddMarker(pos, markType, desc);
			}
		}
		void sendQuestLog() {
			if (client) {
				client->sendQuestLog();
			}
		}
		void sendQuestLine(const Quest* quest) {
			if (client) {
				client->sendQuestLine(quest);
			}
		}
		void sendEnterWorld() {
			if (client) {
				client->sendEnterWorld();
			}
		}
		void sendFightModes() {
			if (client) {
				client->sendFightModes();
			}
		}
		void sendNetworkMessage(const NetworkMessage& message) {
			if (client) {
				client->writeToOutputBuffer(message);
			}
		}

		void receivePing() {
			lastPong = OTSYS_TIME();
		}

		void onThink(uint32_t interval) override;

		void postAddNotification(Thing* thing, const Cylinder* oldParent, int32_t index, cylinderlink_t link = LINK_OWNER) override;
		void postRemoveNotification(Thing* thing, const Cylinder* newParent, int32_t index, cylinderlink_t link = LINK_OWNER) override;

		void setNextAction(int64_t time) {
			if (time > nextAction) {
				nextAction = time;
			}
		}
		bool canDoAction() const {
			return nextAction <= OTSYS_TIME();
		}
		uint32_t getNextActionTime() const;

		void setNextCastMove(int64_t time) {
			nextMove = time;
		}
		bool canCastMove() const {
			if (hasFlag(PlayerFlag_CannotUseMoves)) {
				return false;
			}
			
			return nextMove <= OTSYS_TIME();
		}
		uint32_t getNextCastMoveTime() const;


		void setNextGoback(int64_t time) {
			if (time > nextGoback) {
				nextGoback = time;
			}
		}
		bool canDoGoback() const {
			return nextGoback <= OTSYS_TIME();
		}
		uint32_t getNextGobackTime() const;

		Item* getWriteItem(uint32_t& windowTextId, uint16_t& maxWriteLen);
		void setWriteItem(Item* item, uint16_t maxWriteLen = 0);

		House* getEditHouse(uint32_t& windowTextId, uint32_t& listId);
		void setEditHouse(House* house, uint32_t listId = 0);

		void learnInstantMove(const std::string& moveName);
		void forgetMove(const std::string& moveName);
		bool hasLearnedInstantMove(const std::string& moveName) const;

		Pokemon* getHisPokemon() const {
			if (summons.size()) {
				Creature* creature = summons.front();
				if (creature) {
					return creature->getPokemon();
				}
			}

			return nullptr;
		}

		void tryCatchPokemon(const PokeballType* pokeballType, Item* corpse, Item* pokeball, double rate, const Position& fromPos, const Position& toPos);
		void sendPokemonEmot(uint16_t pokemonEmot) const;
		void sendPokemonTextMessage(const std::string& message) const;
		bool feed(const FoodType* foodType) override;
		bool gobackPokemon(Item* pokeball, bool ignoreDelay = false, bool ignoreTransformPokeball = false);

	private:
		std::forward_list<Condition*> getMuteConditions() const;

		void checkTradeState(const Item* item);
		bool hasCapacity(const Item* item, uint32_t count) const;

		void gainExperience(uint64_t gainExp, Creature* source);
		void addExperience(Creature* source, uint64_t exp, bool sendText = false);
		void removeExperience(uint64_t exp, bool sendText = false);

		void updateInventoryWeight();

		void setNextWalkActionTask(SchedulerTask* task);
		void setNextWalkTask(SchedulerTask* task);
		void setNextActionTask(SchedulerTask* task);

		void death(Creature* lastHitCreature) override;
		bool dropCorpse(Creature* lastHitCreature, Creature* mostDamageCreature, bool lastHitUnjustified, bool mostDamageUnjustified) override;
		Item* getCorpse(Creature* lastHitCreature, Creature* mostDamageCreature) override;

		//cylinder implementations
		ReturnValue queryAdd(int32_t index, const Thing& thing, uint32_t count,
				uint32_t flags, Creature* actor = nullptr) const override;
		ReturnValue queryMaxCount(int32_t index, const Thing& thing, uint32_t count, uint32_t& maxQueryCount,
				uint32_t flags) const override;
		ReturnValue queryRemove(const Thing& thing, uint32_t count, uint32_t flags) const override;
		Cylinder* queryDestination(int32_t& index, const Thing& thing, Item** destItem,
				uint32_t& flags) override;

		void addThing(Thing*) override {}
		void addThing(int32_t index, Thing* thing) override;

		void updateThing(Thing* thing, uint16_t itemId, uint32_t count) override;
		void replaceThing(uint32_t index, Thing* thing) override;

		void removeThing(Thing* thing, uint32_t count) override;

		int32_t getThingIndex(const Thing* thing) const override;
		size_t getFirstIndex() const override;
		size_t getLastIndex() const override;
		uint32_t getItemTypeCount(uint16_t itemId, int32_t subType = -1) const override;
		std::map<uint32_t, uint32_t>& getAllItemTypeCount(std::map<uint32_t, uint32_t>& countMap) const override;
		Thing* getThing(size_t index) const override;

		void internalAddThing(Thing* thing) override;
		void internalAddThing(uint32_t index, Thing* thing) override;

		std::unordered_set<uint32_t> attackedSet;
		std::unordered_set<uint32_t> VIPList;

		std::map<uint8_t, OpenContainer> openContainers;
		std::map<uint32_t, DepotLocker*> depotLockerMap;
		std::map<uint32_t, DepotChest*> depotChests;
		std::map<uint32_t, int32_t> storageMap;

		std::vector<OutfitEntry> outfits;
		GuildWarVector guildWarVector;

		std::list<ShopInfo> shopItemList;

		std::forward_list<Party*> invitePartyList;
		std::forward_list<uint32_t> modalWindows;
		std::forward_list<std::string> learnedInstantMoveList;
		std::forward_list<Condition*> storedConditionList; // TODO: This variable is only temporarily used when logging in, get rid of it somehow

		std::string name;
		std::string guildNick;

		Skill skills[SKILL_LAST + 1];
		LightInfo itemsLight;
		Position loginPosition;
		Position lastWalkthroughPosition;

		time_t lastLoginSaved = 0;
		time_t lastLogout = 0;

		uint64_t experience = 0;
		uint64_t lastAttack = 0;
		uint64_t bankBalance = 0;
		uint64_t lastQuestlogUpdate = 0;
		int64_t lastFailedFollow = 0;
		int64_t lastWalkthroughAttempt = 0;
		int64_t lastToggleMount = 0;
		int64_t lastPing;
		int64_t lastPong;
		int64_t nextAction = 0;
		int64_t nextMove = 0;
		int64_t nextGoback = 0;

		//BedItem* bedItem = nullptr;
		Guild* guild = nullptr;
		const GuildRank* guildRank = nullptr;
		Group* group = nullptr;
		Inbox* inbox;
		Item* tradeItem = nullptr;
 		Item* inventory[CONST_SLOT_LAST + 1] = {};
		Item* writeItem = nullptr;
		House* editHouse = nullptr;
		Npc* shopOwner = nullptr;
		Party* party = nullptr;
		Player* tradePartner = nullptr;
		ProtocolGame_ptr client;
		SchedulerTask* walkTask = nullptr;
		Town* town = nullptr;
		Profession* profession = nullptr;
		Clan* clan = nullptr;

		uint32_t inventoryWeight = 0;
		uint32_t capacity = 40000;
		uint32_t damageImmunities = 0;
		uint32_t conditionImmunities = 0;
		uint32_t conditionSuppressions = 0;
		uint32_t level = 1;
		uint32_t actionTaskEvent = 0;
		uint32_t nextStepEvent = 0;
		uint32_t walkTaskEvent = 0;
		uint32_t MessageBufferTicks = 0;
		uint32_t lastIP = 0;
		uint32_t accountNumber = 0;
		uint32_t guid = 0;
		uint32_t windowTextId = 0;
		uint32_t editListId = 0;
		uint32_t pokemonHealth = 0;
		uint32_t pokemonHealthMax = 0;
		int32_t varSkills[SKILL_LAST + 1] = {};
		int32_t varSpecialSkills[SPECIALSKILL_LAST + 1] = {};
		int32_t varStats[STAT_LAST + 1] = {};
		int32_t purchaseCallback = -1;
		int32_t saleCallback = -1;
		int32_t MessageBufferCount = 0;
		int32_t premiumDays = 0;
		int32_t idleTime = 0;

		uint16_t staminaMinutes = 2520;
		uint16_t maxWriteLen = 0;
		int16_t lastDepotId = -1;

		uint8_t pokemonCapacity = 0;
		uint8_t blessings = 0;
		uint8_t levelPercent = 0;

		PlayerSex_t sex = PLAYERSEX_FEMALE;
		OperatingSystem_t operatingSystem = CLIENTOS_NONE;
		tradestate_t tradeState = TRADE_NONE;
		fightMode_t fightMode = FIGHTMODE_ATTACK;
		AccountType_t accountType = ACCOUNT_TYPE_NORMAL;

		bool chaseMode = false;
		bool inMarket = false;
		bool wasMounted = false;
		bool ghostMode = false;
		bool pzLocked = false;
		bool isConnecting = false;
		bool addAttackSkillPoint = false;
		bool inventoryAbilities[CONST_SLOT_LAST + 1] = {};

		static uint32_t playerAutoID;

		void updateItemsLight(bool internal = false);
		int32_t getStepSpeed() const override {
			return std::max<int32_t>(PLAYER_MIN_SPEED, std::min<int32_t>(PLAYER_MAX_SPEED, getSpeed()));
		}
		void updateBaseSpeed() {
			if (!hasFlag(PlayerFlag_SetMaxSpeed)) {
				baseSpeed = profession->getBaseSpeed() + (2 * (level - 1));
			} else {
				baseSpeed = PLAYER_MAX_SPEED;
			}
		}

		bool isPromoted() const;

		static uint8_t getPercentLevel(uint64_t count, uint64_t nextLevelCount);
		double getLostPercent() const;
		uint64_t getLostExperience() const override {
			return skillLoss ? static_cast<uint64_t>(experience * getLostPercent()) : 0;
		}
		uint32_t getDamageImmunities() const override {
			return damageImmunities;
		}
		uint32_t getConditionImmunities() const override {
			return conditionImmunities;
		}
		uint32_t getConditionSuppressions() const override {
			return conditionSuppressions;
		}
		uint16_t getLookCorpse() const override;
		void getPathSearchParams(const Creature* creature, FindPathParams& fpp) const override;

		friend class Game;
		friend class Npc;
		friend class LuaScriptInterface;
		friend class Map;
		friend class Actions;
		friend class IOLoginData;
		friend class ProtocolGame;
		friend class PokeballType;
		friend class FoodType;
};

#endif
