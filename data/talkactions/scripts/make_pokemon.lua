local string_to_gender = {
["undefined"] =  GENDER_UNDEFINED,
["male"] = GENDER_MALE,
["female"] = GENDER_FEMALE
}


function onSay(player, words, param)
	if not player:getGroup():getAccess() then
		return true
	end

	if player:getAccountType() < ACCOUNT_TYPE_GOD then
		return false
	end

	local split = param:split(",")
	local pokemon_name = split[1]
	local pokeball_type = split[2]
	local pokemon_gender = split[3]
	local gender = string_to_gender[pokemon_gender]

	if not pokemon_name then
		player:sendCancelMessage("Missing Pokemon Name.")
		return false
	elseif not pokeball_type then
		player:sendCancelMessage("Missing Pokeball Type.")
		return false
	elseif not pokemon_gender then
		player:sendCancelMessage("Missing Pokemon Gender.")
		return false
	end

	if not PokemonType(pokemon_name) then
		player:sendCancelMessage("This pokemon doesn't exist.")
		return false
	elseif not PokeballType(pokeball_type) then
		player:sendCancelMessage("Pokeball doesn't exist.")
		return false
	elseif not gender then
		player:sendCancelMessage("gender not found.")
		return false
	end

	
	player:addPokemon(pokemon_name, pokeball_type, gender)
	return false
end
