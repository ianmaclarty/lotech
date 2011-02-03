dofile("../src/ltlua/lt.lua")

local layer0
local world
local main_layer

local t0 = 0
local frames = 0
local function print_fps()
    local t = os.time()
    local dt = os.difftime(t, t0)
    if dt >= 1 then
        print("FPS = " .. frames)
        frames = 0
        t0 = t
    end
    frames = frames + 1
end
local function print_num_wrefs()
    local i = 0
    for _ in next, lt.wrefs do
        i = i + 1
    end
    print(i)
end

lt.SetViewPort(-9, -6, 9, 6)

local function init() 
    world = lt.World()
    layer0 = lt.Layer()
    local floor = world:AddStaticBody()
    floor:AddRect(-20, -10, 20, -7)
    layer0:Insert(floor, 1)

    local things = {}
    local colors = {}
    local n = 2000
    for i = 1, n do
        things[i] = world:AddDynamicBody((i - n / 2) * (10 / n), math.sin(i) * 50 + 60, math.cos(i) * 180)
        local size = (math.random() + 0.5) * 0.25
        things[i]:AddRect(-size, -size, size, size, 3.3)
        local node = lt.Tint(things[i], 0.5, 0.5, 0.5)
        node.on = false
        node.touch_count = 0
        things[i]:OnPointerDown(function(x, y)
            if node.on then
                node.r = 0.5
                node.g = 0.5 + node.touch_count / 10
                node.on = false
            else
                node.r = 1
                node.on = true
                node.touch_count = node.touch_count + 1
            end
            return true
        end)
        things[i]:OnPointerOver(
            function() -- enter
                node.b = 1
                return true
            end,
            function() -- exit
                node.b = 0.5
                return true
            end
        )
        layer0:Insert(node, 1)
    end

    main_layer = lt.Rotate(lt.Translate(lt.Scale(lt.Rotate(lt.Translate(layer0, 3, -0.5), -30), -1.1, 1.1), -1, 0.5), 10)
end
init()

local mouse_x, mouse_y = 0, 0

function lt.Advance()
    local step = lt.secs_per_frame
    world:Step(step)
    main_layer:PropogatePointerMoveEvent(mouse_x, mouse_y)
end

function lt.Render()
    main_layer:Draw()
    --print_fps()
    --print_num_wrefs()
end

function lt.PointerDown(button, x, y)
    main_layer:PropogatePointerDownEvent(button, x, y)
end

function lt.PointerMove(x, y)
    mouse_x = x
    mouse_y = y
end

function lt.KeyDown(key)
    if key == "R" then
        init()
    end
end
