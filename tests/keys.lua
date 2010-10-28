lt.SetViewPort(-10, -10, 10, 10)
lt.SetGravity(0, 0)

local s = lt.StaticBody()
s:AddRect(-10, -10, 10, -9)
s:AddRect(-10, 9, 10, 10)
s:AddRect(-10, -10, -9, 10)
s:AddRect(9, -10, 10, 10)

local b = lt.DynamicBody(0, 0, 0)
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
        b:ApplyTorque(-200.1)
    elseif keys.right then
        b:ApplyTorque(200.1)
    end
    if keys.up then
        b:ApplyForce(math.sin(angle) * 10, math.cos(angle) * 10)
    end

    lt.DoWorldStep()
end

function lt.Render()
    lt.SetColor(1, 0, 0)
    b:DrawShapes()
    lt.SetColor(0, 0, 1)
    s:DrawShapes()
end
