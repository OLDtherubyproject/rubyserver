function onUpdateDatabase()
	print("> Updating database to version 24 (UTF-8 encoding)")
	db.query("ALTER TABLE `pokemon` ADD `shiny` BOOLEAN NOT NULL DEFAULT FALSE AFTER `type`;")	
	return true
end
