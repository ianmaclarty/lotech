dofile("../src/ltlua/lt.lua")

lt.SetViewPort(-10, -10, 10, 10)

local floor = lt.StaticBody()
floor:AddRect(-20, -10, 20, -7)

local things = {}
local colors = {}
local n = 300
for i = 1, n do
    things[i] = lt.DynamicBody((i - n / 2) * (10 / n), math.sin(i) * 50 + 60, math.cos(i) * 180)
    local size = (math.random() + 0.5) * 0.25
    things[i]:AddRect(-size, -size, size, size, 3.3)
    local red = 0
    local blue = 0
    local green = 0.5
    colors[i] = {r = red, b = blue, g = green}
    local function add_tween()
        lt.Tween(colors[i], "r", 1, 0.5, lt.EaseOut,
            function()
                lt.Tween(colors[i], "r", 0, 0.5, lt.EaseIn, add_tween)
            end
        )
    end
    add_tween()
end

function lt.Advance()
    lt.DoWorldStep()
    lt.AdvanceTweens()
end

function lt.Render()
    lt.SetColor(0, 1, 0)
    floor:DrawShapes()
    for i = 1, n do
        lt.SetColor(colors[i].r, colors[i].g, colors[i].b)
        things[i]:DrawShapes()
    end
end
