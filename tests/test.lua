dofile("../src/ltlua/lt.lua")

local scene = lt.Scene()
local tweens = lt.TweenSet()

local t0 = 0;
local frames = 0;
function print_fps()
    local t = os.time()
    local dt = os.difftime(t, t0)
    if dt >= 1 then
        print("FPS = " .. frames)
        frames = 0
        t0 = t
    end
    frames = frames + 1
end

lt.SetViewPort(-10, -10, 10, 10)

local floor = lt.StaticBody()
floor:AddRect(-20, -10, 20, -7)

local things = {}
local colors = {}
local n = 500
for i = 1, n do
    things[i] = lt.DynamicBody((i - n / 2) * (10 / n), math.sin(i) * 50 + 60, math.cos(i) * 180)
    local size = (math.random() + 0.5) * 0.25
    things[i]:AddRect(-size, -size, size, size, 3.3)
    local red = 0
    local blue = 0
    local green = 0.5
    colors[i] = {r = red, b = blue, g = green}
    local function add_tween()
        lt.AddTween(tweens, colors[i], "r", 1, 0.5, lt.EaseOut,
            function()
                lt.AddTween(tweens, colors[i], "r", 0, 0.5, lt.EaseIn, add_tween)
            end
        )
    end
    add_tween()
    function draw()
        lt.SetColor(colors[i].r, colors[i].g, colors[i].b)
        things[i]:DrawShapes()
    end
    lt.AddToScene(scene, draw, 1)
end

function lt.Advance()
    lt.DoWorldStep()
    lt.AdvanceTweens(tweens, 1/60)
end

function lt.Render()
    lt.SetColor(0, 1, 0)
    floor:DrawShapes()
    lt.DrawScene(scene)
    --[[
    for i = 1, n do
        lt.SetColor(colors[i].r, colors[i].g, colors[i].b)
        things[i]:DrawShapes()
    end
    ]]
    print_fps()
end
