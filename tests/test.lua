dofile("../src/ltlua/lt.lua")

local layer0 = lt.Layer()
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

lt.SetViewPort(-9, -6, 9, 6)

local floor = world:StaticBody()
floor:AddRect(-20, -10, 20, -7)
layer0:Insert(floor, 1)

local things = {}
local colors = {}
local n = 500
for i = 1, n do
    things[i] = world:DynamicBody((i - n / 2) * (10 / n), math.sin(i) * 50 + 60, math.cos(i) * 180)
    local size = (math.random() + 0.5) * 0.25
    things[i]:AddRect(-size, -size, size, size, 3.3)
    local node = lt.Tint(things[i], 0.5, 0.5, 0.5)
    things[i]:OnPointerDown(function(x, y)
        node:Set{r = 1}
        return true
    end)
    things[i]:OnPointerOver(
        function() -- enter
            node:Set{b = 1}
            return true
        end,
        function() -- exit
            node:Set{b = 0.5}
            return true
        end
    )
    layer0:Insert(node, 1)
end

local main_layer = lt.Rotate(lt.Translate(lt.Scale(lt.Rotate(lt.Translate(layer0, 3, -0.5), -30), -1.1, 1.1), -1, 0.5), 10)

local mouse_x, mouse_y = 0, 0

function lt.Advance()
    local step = lt.secs_per_frame
    world:Step(step)
    main_layer:PropogatePointerMoveEvent(mouse_x, mouse_y)
end

function lt.Render()
    main_layer:Draw()
    --print_fps()
end

function lt.MouseDown(button, x, y)
    main_layer:PropogatePointerDownEvent(button, x, y)
end

function lt.MouseMove(x, y)
    mouse_x = x
    mouse_y = y
end

