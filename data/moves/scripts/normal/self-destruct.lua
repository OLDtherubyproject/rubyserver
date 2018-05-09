local combat = Combat()
combat:setParameter(COMBAT_PARAM_TYPE, COMBAT_NORMALDAMAGE)
combat:setParameter(COMBAT_PARAM_EFFECT, 947)
combat:setArea(createCombatArea(AREA_CIRCLE3X3))

function onGetFormulaValues(pokemon, level)
	return -((((2 * level) / 5.0) + 2.0) * 200)
end

combat:setCallback(CALLBACK_PARAM_LEVELATTACKVALUE, "onGetFormulaValues")

function onCastMove(creature, variant)
    if combat:execute(creature, variant) then
        creature:kill()
    end
    
    return true
end