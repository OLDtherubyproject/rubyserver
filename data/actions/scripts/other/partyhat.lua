function onUse(player, item, fromPosition, target, toPosition, isHotkey)
	player:getPosition():sendEffect(CONST_ME_GIFT_WRAPS)
	return true
end
