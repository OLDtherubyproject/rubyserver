local condition = Condition(CONDITION_CONFUSION)
condition:setParameter(CONDITION_PARAM_TICKS, 15000)

local combat = Combat()
combat:setParameter(COMBAT_PARAM_EFFECT, 580)
combat:setArea(createCombatArea(AREA_CIRCLE3X3))
combat:addCondition(condition);

function onCastMove(creature, variant)
    return combat:execute(creature, variant)
end