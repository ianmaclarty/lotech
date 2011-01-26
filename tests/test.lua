dofile("../src/ltlua/lt.lua")

local scene = lt.Scene()
local tweens = lt.TweenSet()
local world = lt.World()

local t0 = 0
local frames = 0
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

local floor = world:StaticBody()
floor:AddRect(-20, -10, 20, -7)
scene:Insert(floor, 1)

local things = {}
local colors = {}
local n = 1000
for i = 1, n do
    things[i] = world:DynamicBody((i - n / 2) * (10 / n), math.sin(i) * 50 + 60, math.cos(i) * 180)
    local size = (math.random() + 0.5) * 0.25
    things[i]:AddRect(-size, -size, size, size, 3.3)
    local node = lt.Tint(things[i], 0.5, 0.5, 0.5)
    things[i]:OnPointerDown(function()
        node:Set{r = 1}
    end)
    scene:Insert(node, 1)
end

function lt.Advance()
    local step = lt.secs_per_frame
    world:Step(step)
end

function lt.Render()
    scene:Draw()
    print_fps()
end

function lt.MouseDown(button, x, y)
    scene:PropogatePointerDownEvent(button, x, y)
end

