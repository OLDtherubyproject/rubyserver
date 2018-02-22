function onUse(player, item, fromPosition, target, toPosition, isHotkey)
	local ballrate = balls[item:getId()].rate
	local pokerate = target:getCatchRate()
	local ball = balls[item:getId()]
	local depot = player:getDepotChest()
	local pname = target:getName()
	local bonusvida = target:getMaxHealth()*0.3
	catchball = Game.createItem(ball.normal)
	local catchquery = "INSERT INTO `pokes` (`name`, `vida`, `vidatotal`, `bt`, `level`, `originaltrainer`, `ivatk`, `ivdef`, `ivhp`) VALUES "
	local lastid = 1
	local catchrate = pokerate*ballrate
	if target:getHealth() <= bonusvida then
		local catchrate = catchrate + (catchrate*0.3)
	elseif target:getHealth() == target:getMaxHealth() then
		local catchrate = catchrate - (catchrate*0.3)
	elseif target:getHealth() >= bonusvida and target:getHealth() <= target:getMaxHealth()*0.5 then
		local catchrate = catchrate
		end
	if not target:getMaster() and target:isMonster() and not target:isPlayer() then
		local ac = math.random(1, 1000)
			if ac >= 1 and ac <= catchrate then
				catchball:setAttribute("attack", lastid+1)
				local catchquery = catchquery .. "('"..pname.."', '"..pokes[pname].vida.."', '"..pokes[pname].vida.."', '".. ball.type .. "', '" .. math.random(1, target:getMaxLevel()) .."', '" .. player:getName() .. "', '".. math.random(1, 8) .."', '" .. math.random(1, 8) .. "', '" .. math.random(1, 8) .. "')"
				db.query(catchquery)
				player:sendTextMessage(MESSAGE_STATUS_CONSOLE_ORANGE, "Gotcha!\n Você capturou um ".. pname .. "!")
				target:remove()
				catchball:moveTo(player)
				item:remove(1)
				player:say("Gotcha!", TALKTYPE_ORANGE_1)
				if player:getFreeCapacity() == 0 then
						catchball:moveTo(depot)
					player:sendTextMessage(MESSAGE_STATUS_CONSOLE_ORANGE, "Seu " .. pname .. " foi enviado para o centro pokemon!")
					end
			elseif ac >= catchrate then
				item:remove(1)
					creature:say("Ouch!", TALKTYPE_ORANGE_1)
					player:sendTextMessage(MESSAGE_STATUS_CONSOLE_ORANGE, "Ouch!\n O ".. pname .. " escapou!")
				target:remvove()
		end					
end
	end
