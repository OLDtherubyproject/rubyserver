function onSay(player, words, param)
    if words == '!:@' then
        player:getPosition():sendEffect(CONST_ME_EMOT_ANGRY)
    elseif words == '!:|' then
        player:getPosition():sendEffect(CONST_ME_EMOT_NEUTRAL)
    elseif words == '!pb' then
        player:getPosition():sendEffect(CONST_ME_EMOT_POKEBALL)
    elseif words == '!idea' then
        player:getPosition():sendEffect(CONST_ME_EMOT_IDEA)
    elseif words == '!lol' then
        player:getPosition():sendEffect(CONST_ME_EMOT_LOL)
    elseif words == '!:(' then
        player:getPosition():sendEffect(CONST_ME_EMOT_SAD)
    elseif words == '!xD' then
        player:getPosition():sendEffect(CONST_ME_EMOT_XD)
    elseif words == '!!' then
        player:getPosition():sendEffect(CONST_ME_EMOT_EXCLAMATION)
    elseif words == '!?' then
        player:getPosition():sendEffect(CONST_ME_EMOT_QUESTION)
    elseif words == '!go' then
        player:getPosition():sendEffect(CONST_ME_EMOT_GO)
    elseif words == '!:)' then
        player:getPosition():sendEffect(CONST_ME_EMOT_HAPPY)
    elseif words == '!O.o' then
        player:getPosition():sendEffect(CONST_ME_EMOT_CONFUSED)
    elseif words == '!...' then
        player:getPosition():sendEffect(CONST_ME_EMOT_THREE_POINTS)
    elseif words == '!damn' then
        player:getPosition():sendEffect(CONST_ME_EMOT_WATER)
    elseif words == '!no' then
        player:getPosition():sendEffect(CONST_ME_EMOT_NO)
    elseif words == '!:D' then
        player:getPosition():sendEffect(CONST_ME_EMOT_SOHAPPY)
    elseif words == '!:S' then
        player:getPosition():sendEffect(CONST_ME_EMOT_SEASICK)
    elseif words == '!$' then
        player:getPosition():sendEffect(CONST_ME_EMOT_MONEY)
    elseif words == '!pikachu' then
        player:getPosition():sendEffect(CONST_ME_EMOT_PIKACHU)
    elseif words == '!yes' then
        player:getPosition():sendEffect(CONST_ME_EMOT_YES)
    end

    return false
end