local name = "Blastoise" -- Pokemon Name!
local pokeballType = 27787 -- Pokeball Id!
local gender = {GENDER_UNDEFINED, GENDER_MALE, GENDER_FEMALE} -- Pokemon Genders!
local isShiny = false -- Pokemon is Shiny ?
local nature = NATURE_BRAVE -- Pokemon nature!
------------------------
local ivs = { -- Config!
health = 30, -- Pokemon Health!
attack = 30, -- Pokemon Attack!
defense = 30, -- Pokemon Defense!
speed = 30, -- Pokemon Speed!
special_attack = 30, -- Pokemon Special Attack!
special_defense = 30 -- Pokemon Special Defense!
}

local storage = 3000 -- Storage Quest!

function onUse(player, item, fromPosition, target, toPosition, isHotkey)
	
	if player:getStorageValue(storage) == -1 then
		player:addPokemon(name, pokeballType, gender[math.random(#gender)], isShiny, nature, ivs.health, ivs.attack, ivs.defense, ivs.speed, ivs.special_attack, ivs.special_defense)
		player:sendTextMessage(MESSAGE_INFO_DESCR, "You earned a "..name..".")
		player:setStorageValue(storage, 1)
	else
		player:sendTextMessage(MESSAGE_EVENT_ADVANCE, "Empty.")
	end
	
	return true
end
