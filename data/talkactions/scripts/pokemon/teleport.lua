function onSay(player, words, param)
    for _, summon in ipairs(player:getSummons()) do
        if not summon:hasSpecialAbility(SPECIALABILITY_TELEPORT) then
            player:sendCancelMessage("Your Pokemon cannot use teleport.")
            return false
        end

        local town = Town(param)
        if town then
            local condition = summon:getCondition(CONDITION_MOVECOOLDOWN, CONDITIONID_DEFAULT, -1)
            if condition then
                local remain_minutes = math.floor((condition:getTicks() / 1000) / 60)
                local remain_seconds = math.floor((condition:getTicks() / 1000) % 60)
                local message = "You need to wait "
                
                if remain_minutes > 1 then
                    message = message .. remain_minutes .. " minutes"
                elseif remain_minutes == 1 then
                    message = message .. remain_minutes .. " minute"
                end

                if remain_seconds > 1 then
                    if remain_minutes >= 1 then
                        message = message .. " and "
                    end

                    message = message .. remain_seconds .. " seconds"
                elseif remain_seconds == 1 then
                    if remain_minutes >= 1 then
                        message = message .. " and "
                    end

                    message = message .. remain_seconds .. " second"
                end

                message = message .. " to use teleport again."
                player:sendTextMessage(MESSAGE_STATUS_CONSOLE_BLUE, message)
                return false
            end

            condition = Condition(CONDITION_MOVECOOLDOWN, CONDITIONID_DEFAULT)
            condition:setParameter(CONDITION_PARAM_SUBID, -1)
            condition:setParameter(CONDITION_PARAM_TICKS, 80000)
            summon:addCondition(condition)

            player:say(summon:getName() .. ", teleport to " .. town:getName() .. "!", TALKTYPE_POKEMON_SAY)
            player:teleportTo(town:getPokemonCenterPosition())
            player:getPosition():sendEffect(CONST_ME_TELEPORT)
            summon:teleportTo(town:getPokemonCenterPosition())
            summon:getPosition():sendEffect(CONST_ME_TELEPORT)
        else
            player:sendCancelMessage("Town not found.")
        end
    end
end