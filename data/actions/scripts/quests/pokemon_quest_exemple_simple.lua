local name = "Blastoise" -- Pokemon Name!
local pokeballType = 27787 -- Pokeball Id!
local gender = {GENDER_UNDEFINED, GENDER_MALE, GENDER_FEMALE} -- Pokemon Genders!
local storage = 3001 -- Storage Quest!

function onUse(player, item, fromPosition, target, toPosition, isHotkey)

	if player:getStorageValue(storage) == -1 then
		player:addPokemon(name, pokeballType, gender[math.random(#gender)])
		player:sendTextMessage(MESSAGE_INFO_DESCR, "You earned a "..name..".")
		player:setStorageValue(storage, 1)
	else
		player:sendTextMessage(MESSAGE_EVENT_ADVANCE, "Empty.")
	end

	return true
end
