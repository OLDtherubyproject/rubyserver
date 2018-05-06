local exhaustion = 1000
local seconds = 45

local potions = {
	[8877] = 200, -- Potion, restore 200hp
	[8878] = 500, -- Super Potion, restore 500hp
	[8879] = 2000, -- Hyper Potion, restore 2000hp
}

function onUse(player, item, fromPosition, target, toPosition, isHotkey)
	if type(target) == "userdata" and not target:isPokemon() then
		player:sendCancelMessage("You can only use potion in the pokemon.")
		return true
	end
	
	if target:getMaster() ~= player then
		return false
	end
	
	if player:getExhaustion(exhaustion) > 0 then
		player:sendCancelMessage("You have to wait ".. player:getExhaustion(exhaustion) .." seconds to use this potion.")
		return true
    end
	
	local potion = potions[item:getId()]
	if potion and not (target:getHealth() == target:getMaxHealth())then
		target:addHealth(potion)
		target:getPosition():sendMagicEffect(CONST_ME_MAGIC_RED)
		player:setExhaustion(exhaustion, seconds)
		item:remove(1)
		return true
	else
		player:sendCancelMessage("This Pokemon does not need any potion.")
		return true
	end

	if item:getId() == 8880 then
		target:addHealth(target:getMaxHealth())
		target:getPosition():sendMagicEffect(CONST_ME_MAGIC_BLUE)
		player:setExhaustion(exhaustion, seconds)
		item:remove(1)
		return true
	end
	
	if item:getId() == 8881 then
		target:cleanConditions();
		target:getPosition():sendMagicEffect(CONST_ME_MAGIC_RED)
		player:setExhaustion(exhaustion, seconds)
		item:remove(1)
		return true
	end
	
	return false
end
