function onUse(player, item, fromPosition, target, toPosition, isHotkey)
	item:transform(item:getId() - 1)
	fromPosition:sendEffect(CONST_ME_POFF)
	return true
end
