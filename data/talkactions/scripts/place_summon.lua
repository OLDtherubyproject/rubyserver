function onSay(player, words, param)
	if not player:getGroup():getAccess() then
		return true
	end

	if player:getAccountType() < ACCOUNT_TYPE_GOD then
		return false
	end

	local split = param:split(",")
	local pType = split[1]
	local level = split[2]

	if not level then
		level = player:getLevel()
	end

	local position = player:getPosition()
	local pokemon = Game.createPokemon(pType, level, position)
	if pokemon then
		player:addSummon(pokemon)
		position:sendEffect(CONST_ME_MAGIC_RED)
	else
		player:sendCancelMessage("There is not enough room.")
		position:sendEffect(CONST_ME_POFF)
	end
	return false
end
