local condition = Condition(CONDITION_SLEEP)
condition:setParameter(CONDITION_PARAM_TICKS, 33000)

local combat = Combat()
combat:setParameter(COMBAT_PARAM_TYPE, COMBAT_GRASSDAMAGE)
combat:setParameter(COMBAT_PARAM_EFFECT, 944)
combat:setArea(createCombatArea(AREA_CIRCLE2X2))
combat:addCondition(condition)

function onCastMove(creature, variant)
	return combat:execute(creature, variant)
end