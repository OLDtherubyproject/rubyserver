function onSay(player, words, param)
	if not player:getGroup():getAccess() then
		return true
	end

	local sound = tonumber(param)
	if(sound ~= nil and sound > 0) then
		player:getPosition():sendSound(sound)
	end

	return false
end
