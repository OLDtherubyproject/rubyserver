function onAddItem(moveitem, tileitem, position)
	if moveitem:getId() == 2693 then
		moveitem:transform(2689)
		position:sendEffect(CONST_ME_HITBYFIRE)
	elseif moveitem:getId() == 6277 then
		moveitem:transform(2687, 12)
		position:sendEffect(CONST_ME_HITBYFIRE)
	end
	return true
end
