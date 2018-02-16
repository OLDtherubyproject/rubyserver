# Creating God Account

If you are testing a server locally and just looking to create a new account without having to setup a website, these 2 sql queries will do that for you. This is just to help you speed up your work flow.

The username and password is 1/1.

First thing you want to do is create the account, go into your phpmyadmin and select your tfs database, click on the sql tab and paste this in the box then click the go button.

```sql
INSERT INTO `accounts` (`id`, `name`, `password`, `secret`, `type`, `premdays`, `lastday`, `email`, `creation`) VALUES ('1', '1', '356a192b7913b04c54574d18c28d46e6395428ab', NULL, '5', '365', '0', '', '0');
```

Now click on the sql tab once more and paste this into the box then click the go button.

```sql
INSERT INTO `players` (`id`, `name`, `group_id`, `account_id`, `level`, `vocation`, `health`, `healthmax`, `experience`, `lookbody`, `lookfeet`, `lookhead`, `looklegs`, `looktype`, `lookaddons`, `maglevel`, `mana`, `manamax`, `manaspent`, `soul`, `town_id`, `posx`, `posy`, `posz`, `conditions`, `cap`, `sex`, `lastlogin`, `lastip`, `save`, `skull`, `skulltime`, `lastlogout`, `blessings`, `onlinetime`, `deletion`, `balance`, `offlinetraining_time`, `offlinetraining_skill`, `stamina`, `skill_fist`, `skill_fist_tries`, `skill_club`, `skill_club_tries`, `skill_sword`, `skill_sword_tries`, `skill_axe`, `skill_axe_tries`, `skill_dist`, `skill_dist_tries`, `skill_shielding`, `skill_shielding_tries`, `skill_fishing`, `skill_fishing_tries`) VALUES ('1', 'God', '3', '1', '1', '0', '150', '150', '0', '0', '0', '0', '0', '136', '0', '0', '0', '0', '0', '0', '1', '0', '0', '0', 0x0, '40000', '1', '0', '0', '1', '0', '0', '0', '0', '0', '0', '0', '43200', '-1', '2520', '10', '0', '10', '0', '10', '0', '10', '0', '10', '0', '10', '0', '10', '0');
```

By default group 3 is a god account/character.