function onLogin(player)
	if player:getLastLoginSaved() == 0 then
		player:addItem(2580, 1, false, 1, CONST_SLOT_ROD)
		player:addItem(2513, 1, false, 1, CONST_SLOT_PORTRAIT)
		player:addItem(8820, 1, false, 1, CONST_SLOT_ORDER)
		player:addItem(2273, 1, false, 1, CONST_SLOT_POKEDEX)
		player:addItem(1991, 1, false, 1, CONST_SLOT_BACKPACK)
		player:addItem(27786, 100, false, 1, CONST_SLOT_SUPPORT)
	end
	return true
end
