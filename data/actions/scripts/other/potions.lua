local potions = {
	[8877] = 200,
	[8878] = 500,
	[8879] = 2000,
}

function onUse(player, item, fromPosition, target, toPosition, isHotkey)
	if type(target) == "userdata" and not target:isPokemon() then
		return false
	end

	if target:getMaster() ~= player then
		return false
	end

	local potion = potions[item:getId()]
	if potion then
		target:addHealth(potion)
		item:remove(1)
	end

	if item:getId() == 8880 or item:getId() == 8881 then
		if item:getId() == 8881 then
			target:cleanConditions();
		end

		target:addHealth(target:getMaxHealth())
		item:remove(1)
		return true
	else
		return false
	end
end
