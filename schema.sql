-- phpMyAdmin SQL Dump
-- version 4.5.4.1deb2ubuntu2
-- http://www.phpmyadmin.net
--
-- Host: localhost
-- Generation Time: Jun 14, 2018 at 02:36 PM
-- Server version: 5.7.22-0ubuntu0.16.04.1
-- PHP Version: 7.2.6-1+ubuntu16.04.1+deb.sury.org+1

SET SQL_MODE = "NO_AUTO_VALUE_ON_ZERO";
SET time_zone = "+00:00";


/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET @OLD_CHARACTER_SET_RESULTS=@@CHARACTER_SET_RESULTS */;
/*!40101 SET @OLD_COLLATION_CONNECTION=@@COLLATION_CONNECTION */;
/*!40101 SET NAMES utf8mb4 */;

--
-- Database: `rubyserver`
--

-- --------------------------------------------------------

--
-- Table structure for table `accounts`
--

CREATE TABLE `accounts` (
  `id` int(10) UNSIGNED NOT NULL,
  `name` varchar(191) COLLATE utf8mb4_unicode_ci NOT NULL,
  `email` varchar(191) COLLATE utf8mb4_unicode_ci NOT NULL,
  `password` varchar(191) COLLATE utf8mb4_unicode_ci NOT NULL,
  `secret` varchar(191) COLLATE utf8mb4_unicode_ci DEFAULT NULL,
  `recovery_key` varchar(191) COLLATE utf8mb4_unicode_ci DEFAULT NULL,
  `avatar` int(11) NOT NULL DEFAULT '1',
  `type` int(11) NOT NULL DEFAULT '0',
  `premdays` int(11) NOT NULL DEFAULT '0',
  `lastday` int(11) DEFAULT NULL,
  `points` int(11) NOT NULL DEFAULT '0',
  `points_spent` int(11) NOT NULL DEFAULT '0',
  `remember_token` varchar(100) COLLATE utf8mb4_unicode_ci DEFAULT NULL,
  `created_at` timestamp NULL DEFAULT NULL,
  `updated_at` timestamp NULL DEFAULT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

-- --------------------------------------------------------

--
-- Table structure for table `account_bans`
--

CREATE TABLE `account_bans` (
  `id` int(10) UNSIGNED NOT NULL,
  `account_id` int(10) UNSIGNED NOT NULL,
  `reason` varchar(191) COLLATE utf8mb4_unicode_ci NOT NULL,
  `banned_at` bigint(20) NOT NULL,
  `expires_at` bigint(20) NOT NULL,
  `banned_by` int(10) UNSIGNED NOT NULL,
  `created_at` timestamp NULL DEFAULT NULL,
  `updated_at` timestamp NULL DEFAULT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

-- --------------------------------------------------------

--
-- Table structure for table `account_viplists`
--

CREATE TABLE `account_viplists` (
  `id` int(10) UNSIGNED NOT NULL,
  `account_id` int(10) UNSIGNED NOT NULL,
  `character_id` int(10) UNSIGNED NOT NULL,
  `description` varchar(191) COLLATE utf8mb4_unicode_ci DEFAULT NULL,
  `icon` int(10) UNSIGNED NOT NULL,
  `notify` tinyint(1) NOT NULL DEFAULT '0',
  `created_at` timestamp NULL DEFAULT NULL,
  `updated_at` timestamp NULL DEFAULT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

-- --------------------------------------------------------

--
-- Table structure for table `characters`
--

CREATE TABLE `characters` (
  `id` int(10) UNSIGNED NOT NULL,
  `name` varchar(191) COLLATE utf8mb4_unicode_ci NOT NULL,
  `account_id` int(10) UNSIGNED NOT NULL,
  `group_id` int(10) UNSIGNED NOT NULL DEFAULT '3',
  `level` int(10) UNSIGNED NOT NULL DEFAULT '1',
  `experience` bigint(20) UNSIGNED NOT NULL DEFAULT '0',
  `profession` int(11) NOT NULL DEFAULT '0',
  `clan` int(11) NOT NULL DEFAULT '0',
  `health` int(11) NOT NULL DEFAULT '500',
  `healthmax` int(11) NOT NULL DEFAULT '500',
  `pokemon_capacity` int(11) NOT NULL DEFAULT '6',
  `lookbody` int(11) NOT NULL DEFAULT '1',
  `lookfeet` int(11) NOT NULL DEFAULT '1',
  `lookhead` int(11) NOT NULL DEFAULT '1',
  `looklegs` int(11) NOT NULL DEFAULT '1',
  `looktype` int(11) NOT NULL DEFAULT '1',
  `lookaddons` int(11) NOT NULL DEFAULT '0',
  `maglevel` int(11) NOT NULL DEFAULT '0',
  `town_id` int(11) NOT NULL DEFAULT '1',
  `posx` int(11) NOT NULL DEFAULT '1',
  `posy` int(11) NOT NULL DEFAULT '1',
  `posz` int(11) NOT NULL DEFAULT '1',
  `conditions` blob,
  `cap` int(11) NOT NULL DEFAULT '1000',
  `sex` int(11) NOT NULL DEFAULT '1',
  `lastlogin` bigint(20) NOT NULL DEFAULT '0',
  `lastip` int(11) NOT NULL DEFAULT '0',
  `save` tinyint(1) NOT NULL DEFAULT '1',
  `lastlogout` bigint(20) NOT NULL DEFAULT '0',
  `blessings` int(11) NOT NULL DEFAULT '0',
  `onlinetime` int(11) NOT NULL DEFAULT '0',
  `stamina` int(11) NOT NULL DEFAULT '5000',
  `skill_fist` int(11) NOT NULL DEFAULT '1',
  `skill_fist_tries` int(11) NOT NULL DEFAULT '0',
  `skill_club` int(11) NOT NULL DEFAULT '1',
  `skill_club_tries` int(11) NOT NULL DEFAULT '0',
  `skill_sword` int(11) NOT NULL DEFAULT '1',
  `skill_sword_tries` int(11) NOT NULL DEFAULT '0',
  `skill_axe` int(11) NOT NULL DEFAULT '1',
  `skill_axe_tries` int(11) NOT NULL DEFAULT '0',
  `skill_dist` int(11) NOT NULL DEFAULT '1',
  `skill_dist_tries` int(11) NOT NULL DEFAULT '0',
  `skill_shielding` int(11) NOT NULL DEFAULT '1',
  `skill_shielding_tries` int(11) NOT NULL DEFAULT '0',
  `skill_fishing` int(11) NOT NULL DEFAULT '1',
  `skill_fishing_tries` int(11) NOT NULL DEFAULT '0',
  `deletion` bigint(20) NOT NULL DEFAULT '0',
  `balance` bigint(20) NOT NULL DEFAULT '0',
  `created_at` timestamp NULL DEFAULT NULL,
  `updated_at` timestamp NULL DEFAULT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

--
-- Triggers `characters`
--
DELIMITER $$
CREATE TRIGGER `ondelete_characters` BEFORE DELETE ON `characters` FOR EACH ROW BEGIN
                            UPDATE `houses` SET `owner` = 0 WHERE `owner` = OLD.`id`;
                        END
$$
DELIMITER ;

-- --------------------------------------------------------

--
-- Table structure for table `characters_online`
--

CREATE TABLE `characters_online` (
  `id` int(10) UNSIGNED NOT NULL,
  `character_id` int(10) UNSIGNED NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

-- --------------------------------------------------------

--
-- Table structure for table `character_deaths`
--

CREATE TABLE `character_deaths` (
  `id` int(10) UNSIGNED NOT NULL,
  `character_id` int(10) UNSIGNED NOT NULL,
  `time` int(11) NOT NULL,
  `level` int(11) NOT NULL,
  `killed_by` int(10) UNSIGNED NOT NULL,
  `is_character` tinyint(1) NOT NULL,
  `mostdamage_by` varchar(191) COLLATE utf8mb4_unicode_ci NOT NULL,
  `mostdamage_is_character` tinyint(1) NOT NULL,
  `unjustified` tinyint(1) NOT NULL,
  `mostdamage_unjustified` tinyint(1) NOT NULL,
  `created_at` timestamp NULL DEFAULT NULL,
  `updated_at` timestamp NULL DEFAULT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

-- --------------------------------------------------------

--
-- Table structure for table `character_depotitems`
--

CREATE TABLE `character_depotitems` (
  `id` int(10) UNSIGNED NOT NULL,
  `character_id` int(10) UNSIGNED NOT NULL,
  `pid` int(11) NOT NULL,
  `sid` int(11) NOT NULL,
  `itemtype` int(11) NOT NULL,
  `count` int(11) NOT NULL,
  `attributes` blob NOT NULL,
  `created_at` timestamp NULL DEFAULT NULL,
  `updated_at` timestamp NULL DEFAULT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

-- --------------------------------------------------------

--
-- Table structure for table `character_inboxitems`
--

CREATE TABLE `character_inboxitems` (
  `id` int(10) UNSIGNED NOT NULL,
  `character_id` int(10) UNSIGNED NOT NULL,
  `pid` int(11) NOT NULL,
  `sid` int(11) NOT NULL,
  `itemtype` int(11) NOT NULL,
  `count` int(11) NOT NULL,
  `attributes` blob NOT NULL,
  `created_at` timestamp NULL DEFAULT NULL,
  `updated_at` timestamp NULL DEFAULT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

-- --------------------------------------------------------

--
-- Table structure for table `character_items`
--

CREATE TABLE `character_items` (
  `id` int(10) UNSIGNED NOT NULL,
  `character_id` int(10) UNSIGNED NOT NULL,
  `pid` int(11) NOT NULL,
  `sid` int(11) NOT NULL,
  `itemtype` int(11) NOT NULL,
  `count` int(11) NOT NULL,
  `attributes` blob NOT NULL,
  `created_at` timestamp NULL DEFAULT NULL,
  `updated_at` timestamp NULL DEFAULT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

-- --------------------------------------------------------

--
-- Table structure for table `character_namelocks`
--

CREATE TABLE `character_namelocks` (
  `id` int(10) UNSIGNED NOT NULL,
  `character_id` int(10) UNSIGNED NOT NULL,
  `reason` varchar(191) COLLATE utf8mb4_unicode_ci NOT NULL,
  `namelocked_at` bigint(20) NOT NULL,
  `namelocked_by` int(10) UNSIGNED NOT NULL,
  `created_at` timestamp NULL DEFAULT NULL,
  `updated_at` timestamp NULL DEFAULT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

-- --------------------------------------------------------

--
-- Table structure for table `character_storages`
--

CREATE TABLE `character_storages` (
  `id` int(10) UNSIGNED NOT NULL,
  `character_id` int(10) UNSIGNED NOT NULL,
  `key` int(11) NOT NULL,
  `value` int(11) NOT NULL,
  `created_at` timestamp NULL DEFAULT NULL,
  `updated_at` timestamp NULL DEFAULT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

-- --------------------------------------------------------

--
-- Table structure for table `game`
--

CREATE TABLE `game` (
  `id` int(10) UNSIGNED NOT NULL,
  `key` varchar(191) COLLATE utf8mb4_unicode_ci NOT NULL,
  `value` int(11) NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

-- --------------------------------------------------------

--
-- Table structure for table `groups`
--

CREATE TABLE `groups` (
  `id` int(10) UNSIGNED NOT NULL,
  `name` varchar(191) COLLATE utf8mb4_unicode_ci NOT NULL,
  `flags` bigint(20) NOT NULL,
  `access` tinyint(1) NOT NULL,
  `maxdepotitems` int(11) NOT NULL,
  `maxvipentries` int(11) NOT NULL,
  `created_at` timestamp NULL DEFAULT NULL,
  `updated_at` timestamp NULL DEFAULT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

-- --------------------------------------------------------

--
-- Table structure for table `guilds`
--

CREATE TABLE `guilds` (
  `id` int(10) UNSIGNED NOT NULL,
  `name` varchar(191) COLLATE utf8mb4_unicode_ci NOT NULL,
  `owner_id` int(10) UNSIGNED NOT NULL,
  `motd` varchar(191) COLLATE utf8mb4_unicode_ci NOT NULL,
  `created_at` timestamp NULL DEFAULT NULL,
  `updated_at` timestamp NULL DEFAULT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

--
-- Triggers `guilds`
--
DELIMITER $$
CREATE TRIGGER `oncreate_guilds` AFTER INSERT ON `guilds` FOR EACH ROW BEGIN
                            INSERT INTO `guild_ranks` (`name`, `level`, `guild_id`) VALUES ("the Leader", 3, NEW.`id`);
                            INSERT INTO `guild_ranks` (`name`, `level`, `guild_id`) VALUES ("a Vice-Leader", 2, NEW.`id`);
                            INSERT INTO `guild_ranks` (`name`, `level`, `guild_id`) VALUES ("a Member", 1, NEW.`id`);
                        END
$$
DELIMITER ;

-- --------------------------------------------------------

--
-- Table structure for table `guild_invites`
--

CREATE TABLE `guild_invites` (
  `id` int(10) UNSIGNED NOT NULL,
  `character_id` int(10) UNSIGNED NOT NULL,
  `guild_id` int(10) UNSIGNED NOT NULL,
  `created_at` timestamp NULL DEFAULT NULL,
  `updated_at` timestamp NULL DEFAULT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

-- --------------------------------------------------------

--
-- Table structure for table `guild_memberships`
--

CREATE TABLE `guild_memberships` (
  `id` int(10) UNSIGNED NOT NULL,
  `character_id` int(10) UNSIGNED NOT NULL,
  `guild_id` int(10) UNSIGNED NOT NULL,
  `guild_rank_id` int(10) UNSIGNED NOT NULL,
  `nick` varchar(191) COLLATE utf8mb4_unicode_ci NOT NULL,
  `created_at` timestamp NULL DEFAULT NULL,
  `updated_at` timestamp NULL DEFAULT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

-- --------------------------------------------------------

--
-- Table structure for table `guild_ranks`
--

CREATE TABLE `guild_ranks` (
  `id` int(10) UNSIGNED NOT NULL,
  `guild_id` int(10) UNSIGNED NOT NULL,
  `name` varchar(191) COLLATE utf8mb4_unicode_ci NOT NULL,
  `level` int(11) NOT NULL,
  `created_at` timestamp NULL DEFAULT NULL,
  `updated_at` timestamp NULL DEFAULT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

-- --------------------------------------------------------

--
-- Table structure for table `guild_wars`
--

CREATE TABLE `guild_wars` (
  `id` int(10) UNSIGNED NOT NULL,
  `guild1` int(10) UNSIGNED NOT NULL,
  `guild2` int(10) UNSIGNED NOT NULL,
  `name1` varchar(191) COLLATE utf8mb4_unicode_ci NOT NULL,
  `name2` varchar(191) COLLATE utf8mb4_unicode_ci NOT NULL,
  `status` tinyint(1) NOT NULL,
  `started` bigint(20) NOT NULL,
  `ended` bigint(20) NOT NULL,
  `created_at` timestamp NULL DEFAULT NULL,
  `updated_at` timestamp NULL DEFAULT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

-- --------------------------------------------------------

--
-- Table structure for table `houses`
--

CREATE TABLE `houses` (
  `id` int(10) UNSIGNED NOT NULL,
  `owner` int(10) UNSIGNED NOT NULL,
  `town_id` int(10) UNSIGNED NOT NULL,
  `paid` int(11) NOT NULL,
  `warnings` int(11) NOT NULL,
  `name` varchar(191) COLLATE utf8mb4_unicode_ci NOT NULL,
  `rent` int(11) NOT NULL,
  `bid` int(11) NOT NULL,
  `bid_end` int(11) NOT NULL,
  `last_bid` int(11) NOT NULL,
  `highest_bidder` int(11) NOT NULL,
  `size` int(11) NOT NULL,
  `beds` int(11) NOT NULL,
  `created_at` timestamp NULL DEFAULT NULL,
  `updated_at` timestamp NULL DEFAULT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

-- --------------------------------------------------------

--
-- Table structure for table `house_lists`
--

CREATE TABLE `house_lists` (
  `id` int(10) UNSIGNED NOT NULL,
  `house_id` int(10) UNSIGNED NOT NULL,
  `listid` int(11) NOT NULL,
  `list` varchar(191) COLLATE utf8mb4_unicode_ci NOT NULL,
  `created_at` timestamp NULL DEFAULT NULL,
  `updated_at` timestamp NULL DEFAULT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

-- --------------------------------------------------------

--
-- Table structure for table `ip_bans`
--

CREATE TABLE `ip_bans` (
  `id` int(10) UNSIGNED NOT NULL,
  `ip` int(11) NOT NULL,
  `reason` varchar(191) COLLATE utf8mb4_unicode_ci NOT NULL,
  `banned_at` bigint(20) NOT NULL,
  `expires_at` bigint(20) NOT NULL,
  `banned_by` int(10) UNSIGNED DEFAULT NULL,
  `created_at` timestamp NULL DEFAULT NULL,
  `updated_at` timestamp NULL DEFAULT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

-- --------------------------------------------------------

--
-- Table structure for table `items`
--

CREATE TABLE `items` (
  `id` int(10) UNSIGNED NOT NULL,
  `fromid` int(11) NOT NULL,
  `toid` int(11) DEFAULT NULL,
  `name` varchar(191) COLLATE utf8mb4_unicode_ci NOT NULL,
  `attributes` json DEFAULT NULL,
  `created_at` timestamp NULL DEFAULT NULL,
  `updated_at` timestamp NULL DEFAULT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

-- --------------------------------------------------------

--
-- Table structure for table `market_history`
--

CREATE TABLE `market_history` (
  `id` int(10) UNSIGNED NOT NULL,
  `character_id` int(10) UNSIGNED NOT NULL,
  `sale` tinyint(1) NOT NULL,
  `itemtype` int(11) NOT NULL,
  `amount` int(11) NOT NULL,
  `price` int(11) NOT NULL,
  `expires_at` bigint(20) NOT NULL,
  `inserted` bigint(20) NOT NULL,
  `state` tinyint(1) NOT NULL,
  `created_at` timestamp NULL DEFAULT NULL,
  `updated_at` timestamp NULL DEFAULT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

-- --------------------------------------------------------

--
-- Table structure for table `market_offers`
--

CREATE TABLE `market_offers` (
  `id` int(10) UNSIGNED NOT NULL,
  `character_id` int(10) UNSIGNED NOT NULL,
  `sale` tinyint(1) NOT NULL,
  `itemtype` int(11) NOT NULL,
  `amount` int(11) NOT NULL,
  `created` bigint(20) NOT NULL,
  `anonymous` tinyint(1) NOT NULL,
  `price` int(11) NOT NULL,
  `created_at` timestamp NULL DEFAULT NULL,
  `updated_at` timestamp NULL DEFAULT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

-- --------------------------------------------------------

--
-- Table structure for table `password_resets`
--

CREATE TABLE `password_resets` (
  `email` varchar(191) COLLATE utf8mb4_unicode_ci NOT NULL,
  `token` varchar(191) COLLATE utf8mb4_unicode_ci NOT NULL,
  `created_at` timestamp NULL DEFAULT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

-- --------------------------------------------------------

--
-- Table structure for table `pokemon`
--

CREATE TABLE `pokemon` (
  `id` int(10) UNSIGNED NOT NULL,
  `pokeball` int(11) NOT NULL,
  `type` varchar(191) COLLATE utf8mb4_unicode_ci NOT NULL,
  `shiny` tinyint(1) NOT NULL,
  `nickname` varchar(191) COLLATE utf8mb4_unicode_ci NOT NULL,
  `gender` int(11) NOT NULL,
  `nature` int(11) NOT NULL,
  `hpnow` int(11) NOT NULL,
  `hp` int(11) NOT NULL,
  `atk` int(11) NOT NULL,
  `def` int(11) NOT NULL,
  `speed` int(11) NOT NULL,
  `spatk` int(11) NOT NULL,
  `spdef` int(11) NOT NULL,
  `conditions` blob NOT NULL,
  `moves` blob NOT NULL,
  `created_at` timestamp NULL DEFAULT NULL,
  `updated_at` timestamp NULL DEFAULT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

-- --------------------------------------------------------

--
-- Table structure for table `settings`
--

CREATE TABLE `settings` (
  `id` int(10) UNSIGNED NOT NULL,
  `key` varchar(191) COLLATE utf8mb4_unicode_ci NOT NULL,
  `value` varchar(191) COLLATE utf8mb4_unicode_ci NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

-- --------------------------------------------------------

--
-- Table structure for table `tickets`
--

CREATE TABLE `tickets` (
  `id` int(10) UNSIGNED NOT NULL,
  `created_by` int(10) UNSIGNED NOT NULL,
  `priority` int(11) NOT NULL,
  `department` int(11) NOT NULL,
  `message` text COLLATE utf8mb4_unicode_ci NOT NULL,
  `image` varchar(191) COLLATE utf8mb4_unicode_ci NOT NULL,
  `created_at` timestamp NULL DEFAULT NULL,
  `updated_at` timestamp NULL DEFAULT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

-- --------------------------------------------------------

--
-- Table structure for table `tile_store`
--

CREATE TABLE `tile_store` (
  `id` int(10) UNSIGNED NOT NULL,
  `house_id` int(10) UNSIGNED NOT NULL,
  `data` blob NOT NULL,
  `created_at` timestamp NULL DEFAULT NULL,
  `updated_at` timestamp NULL DEFAULT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

-- --------------------------------------------------------

--
-- Table structure for table `towns`
--

CREATE TABLE `towns` (
  `id` int(10) UNSIGNED NOT NULL,
  `name` varchar(191) COLLATE utf8mb4_unicode_ci NOT NULL,
  `x` int(11) NOT NULL,
  `y` int(11) NOT NULL,
  `z` int(11) NOT NULL,
  `created_at` timestamp NULL DEFAULT NULL,
  `updated_at` timestamp NULL DEFAULT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

--
-- Indexes for dumped tables
--

--
-- Indexes for table `accounts`
--
ALTER TABLE `accounts`
  ADD PRIMARY KEY (`id`),
  ADD UNIQUE KEY `accounts_name_unique` (`name`),
  ADD UNIQUE KEY `accounts_email_unique` (`email`),
  ADD UNIQUE KEY `accounts_recovery_key_unique` (`recovery_key`);

--
-- Indexes for table `account_bans`
--
ALTER TABLE `account_bans`
  ADD PRIMARY KEY (`id`),
  ADD KEY `account_bans_account_id_foreign` (`account_id`),
  ADD KEY `account_bans_banned_by_foreign` (`banned_by`);

--
-- Indexes for table `account_viplists`
--
ALTER TABLE `account_viplists`
  ADD PRIMARY KEY (`id`),
  ADD UNIQUE KEY `account_viplists_character_id_unique` (`character_id`),
  ADD KEY `account_viplists_account_id_foreign` (`account_id`);

--
-- Indexes for table `characters`
--
ALTER TABLE `characters`
  ADD PRIMARY KEY (`id`),
  ADD UNIQUE KEY `characters_name_unique` (`name`),
  ADD KEY `characters_account_id_foreign` (`account_id`),
  ADD KEY `characters_group_id_foreign` (`group_id`);

--
-- Indexes for table `characters_online`
--
ALTER TABLE `characters_online`
  ADD PRIMARY KEY (`id`),
  ADD KEY `characters_online_character_id_foreign` (`character_id`);

--
-- Indexes for table `character_deaths`
--
ALTER TABLE `character_deaths`
  ADD PRIMARY KEY (`id`),
  ADD KEY `character_deaths_character_id_foreign` (`character_id`),
  ADD KEY `character_deaths_killed_by_foreign` (`killed_by`);

--
-- Indexes for table `character_depotitems`
--
ALTER TABLE `character_depotitems`
  ADD PRIMARY KEY (`id`),
  ADD KEY `character_depotitems_character_id_foreign` (`character_id`);

--
-- Indexes for table `character_inboxitems`
--
ALTER TABLE `character_inboxitems`
  ADD PRIMARY KEY (`id`),
  ADD KEY `character_inboxitems_character_id_foreign` (`character_id`);

--
-- Indexes for table `character_items`
--
ALTER TABLE `character_items`
  ADD PRIMARY KEY (`id`),
  ADD KEY `character_items_character_id_foreign` (`character_id`);

--
-- Indexes for table `character_namelocks`
--
ALTER TABLE `character_namelocks`
  ADD PRIMARY KEY (`id`),
  ADD KEY `character_namelocks_character_id_foreign` (`character_id`),
  ADD KEY `character_namelocks_namelocked_by_foreign` (`namelocked_by`);

--
-- Indexes for table `character_storages`
--
ALTER TABLE `character_storages`
  ADD PRIMARY KEY (`id`),
  ADD UNIQUE KEY `character_storages_key_unique` (`key`),
  ADD KEY `character_storages_character_id_foreign` (`character_id`);

--
-- Indexes for table `game`
--
ALTER TABLE `game`
  ADD PRIMARY KEY (`id`);

--
-- Indexes for table `groups`
--
ALTER TABLE `groups`
  ADD PRIMARY KEY (`id`),
  ADD UNIQUE KEY `groups_name_unique` (`name`);

--
-- Indexes for table `guilds`
--
ALTER TABLE `guilds`
  ADD PRIMARY KEY (`id`),
  ADD UNIQUE KEY `guilds_name_unique` (`name`),
  ADD UNIQUE KEY `guilds_owner_id_unique` (`owner_id`);

--
-- Indexes for table `guild_invites`
--
ALTER TABLE `guild_invites`
  ADD PRIMARY KEY (`id`),
  ADD KEY `guild_invites_character_id_foreign` (`character_id`),
  ADD KEY `guild_invites_guild_id_foreign` (`guild_id`);

--
-- Indexes for table `guild_memberships`
--
ALTER TABLE `guild_memberships`
  ADD PRIMARY KEY (`id`),
  ADD KEY `guild_memberships_character_id_foreign` (`character_id`),
  ADD KEY `guild_memberships_guild_id_foreign` (`guild_id`),
  ADD KEY `guild_memberships_guild_rank_id_foreign` (`guild_rank_id`);

--
-- Indexes for table `guild_ranks`
--
ALTER TABLE `guild_ranks`
  ADD PRIMARY KEY (`id`),
  ADD UNIQUE KEY `guild_ranks_name_unique` (`name`),
  ADD KEY `guild_ranks_guild_id_foreign` (`guild_id`);

--
-- Indexes for table `guild_wars`
--
ALTER TABLE `guild_wars`
  ADD PRIMARY KEY (`id`),
  ADD KEY `guild_wars_guild1_foreign` (`guild1`),
  ADD KEY `guild_wars_guild2_foreign` (`guild2`);

--
-- Indexes for table `houses`
--
ALTER TABLE `houses`
  ADD PRIMARY KEY (`id`),
  ADD KEY `houses_owner_foreign` (`owner`),
  ADD KEY `houses_town_id_foreign` (`town_id`);

--
-- Indexes for table `house_lists`
--
ALTER TABLE `house_lists`
  ADD PRIMARY KEY (`id`),
  ADD KEY `house_lists_house_id_foreign` (`house_id`);

--
-- Indexes for table `ip_bans`
--
ALTER TABLE `ip_bans`
  ADD PRIMARY KEY (`id`),
  ADD UNIQUE KEY `ip_bans_ip_unique` (`ip`),
  ADD KEY `ip_bans_banned_by_foreign` (`banned_by`);

--
-- Indexes for table `items`
--
ALTER TABLE `items`
  ADD PRIMARY KEY (`id`),
  ADD UNIQUE KEY `items_fromid_unique` (`fromid`),
  ADD UNIQUE KEY `items_toid_unique` (`toid`);

--
-- Indexes for table `market_history`
--
ALTER TABLE `market_history`
  ADD PRIMARY KEY (`id`),
  ADD KEY `market_history_character_id_foreign` (`character_id`);

--
-- Indexes for table `market_offers`
--
ALTER TABLE `market_offers`
  ADD PRIMARY KEY (`id`),
  ADD KEY `market_offers_character_id_foreign` (`character_id`);

--
-- Indexes for table `password_resets`
--
ALTER TABLE `password_resets`
  ADD KEY `password_resets_email_index` (`email`);

--
-- Indexes for table `pokemon`
--
ALTER TABLE `pokemon`
  ADD PRIMARY KEY (`id`);

--
-- Indexes for table `settings`
--
ALTER TABLE `settings`
  ADD PRIMARY KEY (`id`),
  ADD UNIQUE KEY `settings_key_unique` (`key`),
  ADD UNIQUE KEY `settings_value_unique` (`value`);

--
-- Indexes for table `tickets`
--
ALTER TABLE `tickets`
  ADD PRIMARY KEY (`id`);

--
-- Indexes for table `tile_store`
--
ALTER TABLE `tile_store`
  ADD PRIMARY KEY (`id`);

--
-- Indexes for table `towns`
--
ALTER TABLE `towns`
  ADD PRIMARY KEY (`id`),
  ADD UNIQUE KEY `towns_name_unique` (`name`);

--
-- AUTO_INCREMENT for dumped tables
--

--
-- AUTO_INCREMENT for table `accounts`
--
ALTER TABLE `accounts`
  MODIFY `id` int(10) UNSIGNED NOT NULL AUTO_INCREMENT, AUTO_INCREMENT=2;
--
-- AUTO_INCREMENT for table `account_bans`
--
ALTER TABLE `account_bans`
  MODIFY `id` int(10) UNSIGNED NOT NULL AUTO_INCREMENT;
--
-- AUTO_INCREMENT for table `account_viplists`
--
ALTER TABLE `account_viplists`
  MODIFY `id` int(10) UNSIGNED NOT NULL AUTO_INCREMENT;
--
-- AUTO_INCREMENT for table `characters`
--
ALTER TABLE `characters`
  MODIFY `id` int(10) UNSIGNED NOT NULL AUTO_INCREMENT, AUTO_INCREMENT=2;
--
-- AUTO_INCREMENT for table `characters_online`
--
ALTER TABLE `characters_online`
  MODIFY `id` int(10) UNSIGNED NOT NULL AUTO_INCREMENT;
--
-- AUTO_INCREMENT for table `character_deaths`
--
ALTER TABLE `character_deaths`
  MODIFY `id` int(10) UNSIGNED NOT NULL AUTO_INCREMENT;
--
-- AUTO_INCREMENT for table `character_depotitems`
--
ALTER TABLE `character_depotitems`
  MODIFY `id` int(10) UNSIGNED NOT NULL AUTO_INCREMENT;
--
-- AUTO_INCREMENT for table `character_inboxitems`
--
ALTER TABLE `character_inboxitems`
  MODIFY `id` int(10) UNSIGNED NOT NULL AUTO_INCREMENT;
--
-- AUTO_INCREMENT for table `character_items`
--
ALTER TABLE `character_items`
  MODIFY `id` int(10) UNSIGNED NOT NULL AUTO_INCREMENT;
--
-- AUTO_INCREMENT for table `character_namelocks`
--
ALTER TABLE `character_namelocks`
  MODIFY `id` int(10) UNSIGNED NOT NULL AUTO_INCREMENT;
--
-- AUTO_INCREMENT for table `character_storages`
--
ALTER TABLE `character_storages`
  MODIFY `id` int(10) UNSIGNED NOT NULL AUTO_INCREMENT;
--
-- AUTO_INCREMENT for table `game`
--
ALTER TABLE `game`
  MODIFY `id` int(10) UNSIGNED NOT NULL AUTO_INCREMENT, AUTO_INCREMENT=2;
--
-- AUTO_INCREMENT for table `groups`
--
ALTER TABLE `groups`
  MODIFY `id` int(10) UNSIGNED NOT NULL AUTO_INCREMENT, AUTO_INCREMENT=4;
--
-- AUTO_INCREMENT for table `guilds`
--
ALTER TABLE `guilds`
  MODIFY `id` int(10) UNSIGNED NOT NULL AUTO_INCREMENT;
--
-- AUTO_INCREMENT for table `guild_invites`
--
ALTER TABLE `guild_invites`
  MODIFY `id` int(10) UNSIGNED NOT NULL AUTO_INCREMENT;
--
-- AUTO_INCREMENT for table `guild_memberships`
--
ALTER TABLE `guild_memberships`
  MODIFY `id` int(10) UNSIGNED NOT NULL AUTO_INCREMENT;
--
-- AUTO_INCREMENT for table `guild_ranks`
--
ALTER TABLE `guild_ranks`
  MODIFY `id` int(10) UNSIGNED NOT NULL AUTO_INCREMENT;
--
-- AUTO_INCREMENT for table `guild_wars`
--
ALTER TABLE `guild_wars`
  MODIFY `id` int(10) UNSIGNED NOT NULL AUTO_INCREMENT;
--
-- AUTO_INCREMENT for table `houses`
--
ALTER TABLE `houses`
  MODIFY `id` int(10) UNSIGNED NOT NULL AUTO_INCREMENT;
--
-- AUTO_INCREMENT for table `house_lists`
--
ALTER TABLE `house_lists`
  MODIFY `id` int(10) UNSIGNED NOT NULL AUTO_INCREMENT;
--
-- AUTO_INCREMENT for table `ip_bans`
--
ALTER TABLE `ip_bans`
  MODIFY `id` int(10) UNSIGNED NOT NULL AUTO_INCREMENT;
--
-- AUTO_INCREMENT for table `items`
--
ALTER TABLE `items`
  MODIFY `id` int(10) UNSIGNED NOT NULL AUTO_INCREMENT;
--
-- AUTO_INCREMENT for table `market_history`
--
ALTER TABLE `market_history`
  MODIFY `id` int(10) UNSIGNED NOT NULL AUTO_INCREMENT;
--
-- AUTO_INCREMENT for table `market_offers`
--
ALTER TABLE `market_offers`
  MODIFY `id` int(10) UNSIGNED NOT NULL AUTO_INCREMENT;
--
-- AUTO_INCREMENT for table `pokemon`
--
ALTER TABLE `pokemon`
  MODIFY `id` int(10) UNSIGNED NOT NULL AUTO_INCREMENT;
--
-- AUTO_INCREMENT for table `settings`
--
ALTER TABLE `settings`
  MODIFY `id` int(10) UNSIGNED NOT NULL AUTO_INCREMENT;
--
-- AUTO_INCREMENT for table `tickets`
--
ALTER TABLE `tickets`
  MODIFY `id` int(10) UNSIGNED NOT NULL AUTO_INCREMENT;
--
-- AUTO_INCREMENT for table `tile_store`
--
ALTER TABLE `tile_store`
  MODIFY `id` int(10) UNSIGNED NOT NULL AUTO_INCREMENT;
--
-- AUTO_INCREMENT for table `towns`
--
ALTER TABLE `towns`
  MODIFY `id` int(10) UNSIGNED NOT NULL AUTO_INCREMENT, AUTO_INCREMENT=2;
--
-- Constraints for dumped tables
--

--
-- Constraints for table `account_bans`
--
ALTER TABLE `account_bans`
  ADD CONSTRAINT `account_bans_account_id_foreign` FOREIGN KEY (`account_id`) REFERENCES `accounts` (`id`) ON DELETE CASCADE,
  ADD CONSTRAINT `account_bans_banned_by_foreign` FOREIGN KEY (`banned_by`) REFERENCES `characters` (`id`) ON DELETE CASCADE;

--
-- Constraints for table `account_viplists`
--
ALTER TABLE `account_viplists`
  ADD CONSTRAINT `account_viplists_account_id_foreign` FOREIGN KEY (`account_id`) REFERENCES `accounts` (`id`) ON DELETE CASCADE,
  ADD CONSTRAINT `account_viplists_character_id_foreign` FOREIGN KEY (`character_id`) REFERENCES `characters` (`id`) ON DELETE CASCADE;

--
-- Constraints for table `characters`
--
ALTER TABLE `characters`
  ADD CONSTRAINT `characters_account_id_foreign` FOREIGN KEY (`account_id`) REFERENCES `accounts` (`id`) ON DELETE CASCADE,
  ADD CONSTRAINT `characters_group_id_foreign` FOREIGN KEY (`group_id`) REFERENCES `groups` (`id`) ON DELETE CASCADE;

--
-- Constraints for table `characters_online`
--
ALTER TABLE `characters_online`
  ADD CONSTRAINT `characters_online_character_id_foreign` FOREIGN KEY (`character_id`) REFERENCES `characters` (`id`) ON DELETE CASCADE;

--
-- Constraints for table `character_deaths`
--
ALTER TABLE `character_deaths`
  ADD CONSTRAINT `character_deaths_character_id_foreign` FOREIGN KEY (`character_id`) REFERENCES `characters` (`id`) ON DELETE CASCADE,
  ADD CONSTRAINT `character_deaths_killed_by_foreign` FOREIGN KEY (`killed_by`) REFERENCES `characters` (`id`) ON DELETE CASCADE;

--
-- Constraints for table `character_depotitems`
--
ALTER TABLE `character_depotitems`
  ADD CONSTRAINT `character_depotitems_character_id_foreign` FOREIGN KEY (`character_id`) REFERENCES `characters` (`id`) ON DELETE CASCADE;

--
-- Constraints for table `character_inboxitems`
--
ALTER TABLE `character_inboxitems`
  ADD CONSTRAINT `character_inboxitems_character_id_foreign` FOREIGN KEY (`character_id`) REFERENCES `characters` (`id`) ON DELETE CASCADE;

--
-- Constraints for table `character_items`
--
ALTER TABLE `character_items`
  ADD CONSTRAINT `character_items_character_id_foreign` FOREIGN KEY (`character_id`) REFERENCES `characters` (`id`) ON DELETE CASCADE;

--
-- Constraints for table `character_namelocks`
--
ALTER TABLE `character_namelocks`
  ADD CONSTRAINT `character_namelocks_character_id_foreign` FOREIGN KEY (`character_id`) REFERENCES `characters` (`id`) ON DELETE CASCADE,
  ADD CONSTRAINT `character_namelocks_namelocked_by_foreign` FOREIGN KEY (`namelocked_by`) REFERENCES `characters` (`id`) ON DELETE CASCADE;

--
-- Constraints for table `character_storages`
--
ALTER TABLE `character_storages`
  ADD CONSTRAINT `character_storages_character_id_foreign` FOREIGN KEY (`character_id`) REFERENCES `characters` (`id`) ON DELETE CASCADE;

--
-- Constraints for table `guilds`
--
ALTER TABLE `guilds`
  ADD CONSTRAINT `guilds_owner_id_foreign` FOREIGN KEY (`owner_id`) REFERENCES `characters` (`id`) ON DELETE CASCADE;

--
-- Constraints for table `guild_invites`
--
ALTER TABLE `guild_invites`
  ADD CONSTRAINT `guild_invites_character_id_foreign` FOREIGN KEY (`character_id`) REFERENCES `characters` (`id`) ON DELETE CASCADE,
  ADD CONSTRAINT `guild_invites_guild_id_foreign` FOREIGN KEY (`guild_id`) REFERENCES `guilds` (`id`) ON DELETE CASCADE;

--
-- Constraints for table `guild_memberships`
--
ALTER TABLE `guild_memberships`
  ADD CONSTRAINT `guild_memberships_character_id_foreign` FOREIGN KEY (`character_id`) REFERENCES `characters` (`id`) ON DELETE CASCADE,
  ADD CONSTRAINT `guild_memberships_guild_id_foreign` FOREIGN KEY (`guild_id`) REFERENCES `guilds` (`id`) ON DELETE CASCADE,
  ADD CONSTRAINT `guild_memberships_guild_rank_id_foreign` FOREIGN KEY (`guild_rank_id`) REFERENCES `guild_ranks` (`id`) ON DELETE CASCADE;

--
-- Constraints for table `guild_ranks`
--
ALTER TABLE `guild_ranks`
  ADD CONSTRAINT `guild_ranks_guild_id_foreign` FOREIGN KEY (`guild_id`) REFERENCES `guilds` (`id`) ON DELETE CASCADE;

--
-- Constraints for table `guild_wars`
--
ALTER TABLE `guild_wars`
  ADD CONSTRAINT `guild_wars_guild1_foreign` FOREIGN KEY (`guild1`) REFERENCES `guilds` (`id`) ON DELETE CASCADE,
  ADD CONSTRAINT `guild_wars_guild2_foreign` FOREIGN KEY (`guild2`) REFERENCES `guilds` (`id`) ON DELETE CASCADE;

--
-- Constraints for table `houses`
--
ALTER TABLE `houses`
  ADD CONSTRAINT `houses_owner_foreign` FOREIGN KEY (`owner`) REFERENCES `characters` (`id`) ON DELETE CASCADE,
  ADD CONSTRAINT `houses_town_id_foreign` FOREIGN KEY (`town_id`) REFERENCES `towns` (`id`) ON DELETE NO ACTION;

--
-- Constraints for table `house_lists`
--
ALTER TABLE `house_lists`
  ADD CONSTRAINT `house_lists_house_id_foreign` FOREIGN KEY (`house_id`) REFERENCES `houses` (`id`) ON DELETE CASCADE;

--
-- Constraints for table `ip_bans`
--
ALTER TABLE `ip_bans`
  ADD CONSTRAINT `ip_bans_banned_by_foreign` FOREIGN KEY (`banned_by`) REFERENCES `characters` (`id`) ON DELETE SET NULL;

--
-- Constraints for table `market_history`
--
ALTER TABLE `market_history`
  ADD CONSTRAINT `market_history_character_id_foreign` FOREIGN KEY (`character_id`) REFERENCES `characters` (`id`) ON DELETE CASCADE;

--
-- Constraints for table `market_offers`
--
ALTER TABLE `market_offers`
  ADD CONSTRAINT `market_offers_character_id_foreign` FOREIGN KEY (`character_id`) REFERENCES `characters` (`id`) ON DELETE CASCADE;

/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
