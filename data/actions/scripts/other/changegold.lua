local config = {
	[ITEM_COIN] = {changeTo = ITEM_DOLLAR_NOTE},
	[ITEM_DOLLAR_NOTE] = {changeBack = ITEM_COIN, changeTo = ITEM_HUNDRED_DOLLAR_NOTE},
	[ITEM_HUNDRED_DOLLAR_NOTE] = {changeBack = ITEM_DOLLAR_NOTE, changeTo = ITEM_TEN_THOUSAND_DOLLAR_NOTE},
	[ITEM_TEN_THOUSAND_DOLLAR_NOTE] = {changeBack = ITEM_HUNDRED_DOLLAR_NOTE, changeTo = ITEM_MILLION_DOLLAR_NOTE},
	[ITEM_MILLION_DOLLAR_NOTE] = {changeBack = ITEM_TEN_THOUSAND_DOLLAR_NOTE}
}

function onUse(player, item, fromPosition, target, toPosition, isHotkey)
	local coin = config[item:getId()]
	if coin.changeTo and item.type == 100 then
		item:remove()
		player:addItem(coin.changeTo, 1)
	elseif coin.changeBack then
		item:remove(1)
		player:addItem(coin.changeBack, 100)
	else
		return false
	end
	return true
end
