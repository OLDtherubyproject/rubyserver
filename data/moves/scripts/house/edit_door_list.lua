function onCastMove(creature, variant)
	local house = creature:getTile():getHouse()
	local doorId = house and house:getDoorIdByPosition(variant:getPosition())
	if not doorId or not house:canEditAccessList(doorId, creature) then
		creature:sendCancelMessage(RETURNVALUE_NOTPOSSIBLE)
		creature:getPosition():sendEffect(CONST_ME_POFF)
		return false
	end

	creature:setEditHouse(house, doorId)
	creature:sendHouseWindow(house, doorId)
	return true
end
