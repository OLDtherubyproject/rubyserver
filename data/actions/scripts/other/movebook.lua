function onUse(player, item, fromPosition, target, toPosition, isHotkey)
	local text = ""
	local moves = {}
	for _, move in ipairs(player:getInstantMoves()) do
		if move.level ~= 0 then
			if move.manapercent > 0 then
				move.mana = move.manapercent .. "%"
			end
			moves[#moves + 1] = move
		end
	end

	table.sort(moves, function(a, b) return a.level < b.level end)

	local prevLevel = -1
	for i, move in ipairs(moves) do
		local line = ""
		if prevLevel ~= move.level then
			if i ~= 1 then
				line = "\n"
			end
			line = line .. "Moves for Level " .. move.level .. "\n"
			prevLevel = move.level
		end
		text = text .. line .. "  " .. move.words .. " - " .. move.name .. " : " .. move.mana .. "\n"
	end

	player:showTextDialog(item:getId(), text)
	return true
end
