local node = lt.Layer()
local rect = lt.Rect(-10, -10, 10, 10):Translate(-200, 0)
node:Insert(rect)
local
function move()
    rect:Tween{x = 200, time = 3, action = function()
        rect:Tween{x = -200, time = 3, action = move}
    end}
end
move()
lt.root.child = node

local n = 0
local
function do_req()
    local req = lt.HTTPRequest("http://www.muandheyo.com/press/images/05ian.txt")

    node:Action(function(dt)
        req:Poll()
        if req.success then
            log(n .. " success: " .. req.response)
            do_req()
            return true
        elseif req.failure then
            log(n .. " failure: " .. req.error)
            do_req()
            return true
        end
    end)

    n = n + 1
end
do_req()
