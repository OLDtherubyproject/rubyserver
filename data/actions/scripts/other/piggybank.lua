function onUse(player, item, fromPosition, target, toPosition, isHotkey)
	if math.random(6) == 1 then
		item:getPosition():sendEffect(CONST_ME_POFF)
		player:addItem(ITEM_COIN, 1)
		item:transform(2115)
	else
		item:getPosition():sendEffect(CONST_ME_SOUND_YELLOW)
		player:addItem(ITEM_DOLLAR_NOTE, 1)
	end
	return true
end
