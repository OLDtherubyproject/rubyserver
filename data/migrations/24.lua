function onUpdateDatabase()
	print("> Updating database to version 25 (remove mana)")
	db.query("ALTER TABLE `players` DROP `mana`, DROP `manamax`, DROP `manaspent`;")	
	return true
end
