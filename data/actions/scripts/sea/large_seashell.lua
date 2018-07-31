local pearls = {7632, 7633}

function onUse(player, item, fromPosition, target, toPosition, isHotkey)
    item:transform(7553)
    if math.random(1, 100) <= 10 then
        player:addItem(pearls[math.random(1, 2)], 1)
        player:sendTextMessage(MESSAGE_INFO_DESCR, "You found 1x giant shimmering pearl.")
    elseif math.random(1, 100) <= 20 then
        player:addHealth(-200)
    end
    return true
end