local combat = Combat()
combat:setParameter(COMBAT_PARAM_TYPE, COMBAT_GRASSDAMAGE)
combat:setParameter(COMBAT_PARAM_SOUND, CONST_SE_VINEWHIP)
combat:setArea(createCombatArea(AREA_WAVE3, AREA_WAVE3))

function onGetFormulaValues(pokemon, level)
	return -((((2 * level) / 5.0) + 2.0) * 40)
end

combat:setCallback(CALLBACK_PARAM_LEVELATTACKVALUE, "onGetFormulaValues")

local effects = {
	[0] = 337,
	[1] = 340,
	[2] = 338,
	[3] = 339
}

local positions = {
	[0] = Position(1, -1, 0),
	[1] = Position(2, 1, 0),
	[2] = Position(1, 2, 0),
	[3] = Position(-1, 1, 0)
}

function onCastMove(creature, variant)
	if combat:execute(creature, variant) then
		dir = creature:getDirection()
		pos = creature:getPosition()
		pos = pos + positions[dir]
		pos:sendEffect(effects[dir])
		return true
	end

	return false
end