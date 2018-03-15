function onUpdateDatabase()
	print("> Updating database to version 26 (add pokemon capacity)")
	db.query("ALTER TABLE `players` ADD `pokemon_capacity` INT(10) UNSIGNED NOT NULL DEFAULT '0' AFTER `healthmax`;")
	return true
end
