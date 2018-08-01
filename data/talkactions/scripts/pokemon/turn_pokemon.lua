function onSay(player, words, param)
    for _, summon in ipairs(player:getSummons()) do
        local directionName
        local direction
        if words == "t1" then
            directionName = "north"
            direction = NORTH
        elseif words == "t2" then
            directionName = "east"
            direction = EAST
        elseif words == "t3" then
            directionName = "south"
            direction = SOUTH
        elseif words == "t4" then
            directionName = "west"
            direction = WEST
        else
            return false
        end

        player:say(summon:getName() .. ", Turn " .. directionName .. "!", TALKTYPE_POKEMON_SAY)
        summon:setDirection(direction)
        return false
    end
end