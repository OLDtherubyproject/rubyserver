function onUse(player, item, fromPosition, target, toPosition, isHotkey)
	if fromPosition.x ~= CONTAINER_POSITION then
		fromPosition:sendEffect(math.random(CONST_ME_FIREWORK_YELLOW, CONST_ME_FIREWORK_BLUE))
	else
		local position = player:getPosition()
		position:sendEffect(CONST_ME_HITBYFIRE)
		position:sendEffect(CONST_ME_EXPLOSIONAREA)
		player:say("Ouch! Rather place it on the ground next time.", TALKTYPE_POKEMON_SAY)
		player:addHealth(-10)
	end

	item:remove()
	return true
end
