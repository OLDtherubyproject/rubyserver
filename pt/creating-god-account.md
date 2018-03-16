# Criando Conta God

Se você estiver testando um servidor localmente e quiser criar uma conta God sem instalar e configurar um website, estas 2 querys sql irão fazer isto para você. Isto é apenas para ajudar você com o seu fluxo de trabalho.

O login e senha é 1/1.

A primeira coisa que você precisa fazer é criar uma conta, para isso vá até o seu phpmyadmin e selecione o banco de dados de seu servidor, clique na aba sql e cole o código abaixo e então clique no botão executar.

```sql
INSERT INTO `accounts` (`id`, `name`, `password`, `secret`, `type`, `premdays`, `lastday`, `email`, `creation`) VALUES ('1', '1', '356a192b7913b04c54574d18c28d46e6395428ab', NULL, '5', '365', '0', '', '0');
```

Agora clique na aba sql mais uma vez e cole o código abaixo e então clique no botão executar.

```sql
INSERT INTO `players` (`id`, `name`, `group_id`, `account_id`, `level`, `vocation`, `health`, `healthmax`, `pokemon_capacity`, `experience`, `lookbody`, `lookfeet`, `lookhead`, `looklegs`, `looktype`, `lookaddons`, `maglevel`, `town_id`, `posx`, `posy`, `posz`, `conditions`, `cap`, `sex`, `lastlogin`, `lastip`, `save`, `skull`, `skulltime`, `lastlogout`, `blessings`, `onlinetime`, `deletion`, `balance`, `offlinetraining_time`, `offlinetraining_skill`, `stamina`, `skill_fist`, `skill_fist_tries`, `skill_club`, `skill_club_tries`, `skill_sword`, `skill_sword_tries`, `skill_axe`, `skill_axe_tries`, `skill_dist`, `skill_dist_tries`, `skill_shielding`, `skill_shielding_tries`, `skill_fishing`, `skill_fishing_tries`) VALUES ('1', 'God', '3', '1', '1', '0', '0', '150', '150', '0', '0', '0', '0', '0', '136', '0', '0', '1', '0', '0', '0', 0x0, '40000', '1', '0', '0', '1', '0', '0', '0', '0', '0', '0', '0', '43200', '-1', '2520', '10', '0', '10', '0', '10', '0', '10', '0', '10', '0', '10', '0', '10', '0');
```

Por padrão o tipo 5 é uma conta do tipo God e o grupo 3 é um Player do tipo God.
