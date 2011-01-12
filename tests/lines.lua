dofile("../src/ltlua/lt.lua")

local bottom = -10
local top = 10
local left = -15
local right = 15
local grid_gap = 0.2
lt.SetViewPort(left, bottom, right, top)

local x1 = 0
local y1 = 0
local curr_line = nil
local down = false

local function grey()
    return 0.1, 0.1, 0.1
end

local scene = lt.Scene()

-- Draw grid lines
local grid = lt.Scene()
for y = bottom, top, grid_gap do
    grid:Insert(lt.Tinter(lt.Line(left, y, right, y), grey()), 0)
end
for x = left, right, grid_gap do
    grid:Insert(lt.Tinter(lt.Line(x, bottom, x, top), grey()), 0)
end
scene:Insert(grid, 0)

local function nearest_grid_point(x, y)
    local nx = math.floor(x / grid_gap + 0.5) * grid_gap;
    local ny = math.floor(y / grid_gap + 0.5) * grid_gap;
    return nx, ny
end

function lt.MouseMove(x, y)
    if down and curr_line then
        scene:Remove(curr_line)
        x, y = nearest_grid_point(x, y)
        curr_line = lt.Line(x1, y1, x, y)
        scene:Insert(curr_line, 1)
    end
end

function lt.MouseDown(button, x, y)
    if button == 1 then
        down = true
        x1, y1 = nearest_grid_point(x, y)
        curr_line = lt.Line(x1, y1, x1, y1)
        scene:Insert(curr_line, 1)
    end
end

function lt.MouseUp(button, x, y)
    if button == 1 then
        down = false
    end
end

function lt.Render()
    scene:Draw()
end
