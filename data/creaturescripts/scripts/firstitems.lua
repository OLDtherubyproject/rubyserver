function onLogin(player)
	if player:getLastLoginSaved() == 0 then
		player:addItem(2513, 1)
		player:addItem(1988, 1)
		player:addItem(27786, 100)
	end
	return true
end
