dofile("../src/ltlua/lt.lua")

lt.SetViewPort(-10, -10, 10, 10)

local w = lt.World()
w:SetGravity(0, -10)
local s = w:StaticBody()
s:AddRect(-10, -10, 10, -9)
s:AddRect(-10, 9, 10, 10)
s:AddRect(-10, -10, -9, 10)
s:AddRect(9, -10, 10, 10)

local b = w:DynamicBody(0, 0, 0)
b:AddRect(-0.5, -0.8, 0.5, 0.8, 10)
local keys = {}

function lt.KeyDown(key)
    keys[key] = true
end

function lt.KeyUp(key)
    keys[key] = false
end

function lt.Advance()
    local angle = b:GetAngle()
    if keys.left then
        --b:ApplyTorque(-20.1)
        b:SetAngle(angle + 1.5)
    elseif keys.right then
        --b:ApplyTorque(20.1)
        b:SetAngle(angle - 1.5)
    end
    if keys.up then
        b:ApplyForce(sin(-angle) * 400, cos(-angle) * 400)
    end

    w:Step(lt.secs_per_frame)
end

function lt.Render()
    lt.SetColor(1, 0, 0)
    b:DrawShapes()
    lt.SetColor(0, 0, 1)
    s:DrawShapes()
end
