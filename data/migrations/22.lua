function onUpdateDatabase()
	print("> Updating database to version 22 (UTF-8 encoding)")
	db.query("CREATE TABLE IF NOT EXISTS `pokemon` (`id` int(11) NOT NULL AUTO_INCREMENT, `type` varchar(255) NOT NULL,`nickname` varchar(255) NOT NULL,`gender` int(1) NOT NULL DEFAULT '0', `nature` int(2) NOT NULL DEFAULT '0', `conditions` blob NOT NULL, PRIMARY KEY (`id`)) ENGINE=InnoDB DEFAULT CHARACTER SET=utf8;")	

	return true
end
