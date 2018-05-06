function Player.setExhaustion(self, value, time)
    self:setStorageValue(value, time + os.time())
end
 
function Player.getExhaustion(self, value)
    local storage = self:getStorageValue(value)
    if not storage or storage <= os.time() then
        return 0
    end
 
    return storage - os.time()
end
 
function Player:hasExhaustion(value)
    return self:getExhaustion(value) >= os.time() and true or false
end