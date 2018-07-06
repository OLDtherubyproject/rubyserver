local config = {
	[ITEM_COIN] = {changeTo = ITEM_DOLLAR_NOTE},
	[ITEM_DOLLAR_NOTE] = {changeBack = ITEM_COIN, changeTo = ITEM_HUNDRED_DOLLAR_NOTE},
	[ITEM_HUNDRED_DOLLAR_NOTE] = {changeBack = ITEM_DOLLAR_NOTE, changeTo = ITEM_TEN_THOUSAND_DOLLAR_NOTE},
	[ITEM_TEN_THOUSAND_DOLLAR_NOTE] = {changeBack = ITEM_HUNDRED_DOLLAR_NOTE, changeTo = ITEM_MILLION_DOLLAR_NOTE},
	[ITEM_MILLION_DOLLAR_NOTE] = {changeBack = ITEM_TEN_THOUSAND_DOLLAR_NOTE}
}

function onUse(player, item, fromPosition, target, toPosition, isHotkey)
	local coin = config[item:getId()]
	if coin.changeTo and item.type >= 100 then
		local m = item.type
		local n = 0
		while m >= 100 do
			m = m - 100
			n = n + 1
		end
		item:remove(item.type - m)
		player:addItem(coin.changeTo, n)
	elseif coin.changeBack then
		item:remove(1)
		player:addItem(coin.changeBack, 100)
	else
		return false
	end
	return true
end
