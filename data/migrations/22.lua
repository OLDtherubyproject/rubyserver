function onUpdateDatabase()
	print("> Updating database to version 23 (add pokemon)")
	db.query("CREATE TABLE IF NOT EXISTS `pokemon`( `id` int(11) NOT NULL AUTO_INCREMENT, `pokeball` int(11) NOT NULL, `type` varchar(255) NOT NULL, `nickname` varchar(255) NOT NULL, `gender` int(1) NOT NULL DEFAULT '0', `nature` int(2) NOT NULL DEFAULT '0', `hpnow` int(11) NOT NULL DEFAULT '0', `hp` int(2) NOT NULL DEFAULT '0', `atk` int(2) NOT NULL DEFAULT '0', `def` int(2) NOT NULL DEFAULT '0', `speed` int(2) NOT NULL DEFAULT '0', `spatk` int(2) NOT NULL DEFAULT '0', `spdef` int(2) NOT NULL DEFAULT '0', `conditions` blob NOT NULL, PRIMARY KEY (`id`)) ENGINE=InnoDB DEFAULT CHARACTER SET=utf8;")	

	return true
end
