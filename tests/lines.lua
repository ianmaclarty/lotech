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

local world = lt.World()
world:SetGravity(0, -10)

local static = world:StaticBody()
static:AddRect(left, bottom, right, bottom + 0.5)
static:AddRect(left, top - 0.5, right, top)
static:AddRect(left, bottom, left + 0.5, top)
static:AddRect(right - 0.5, bottom, right, top)

local body = world:DynamicBody(0, 0, 0)
body:AddTriangle(-0.5, -0.8, 0.5, -0.8, 0, 0.8, 1)
--body:AddRect(-0.5, -0.8, 0.5, 0.8, 1)

local keys = {}

function lt.KeyDown(key)
    keys[key] = true
    if key == "left" then
        body:SetAngularVelocity(130)
    elseif key == "right" then
        body:SetAngularVelocity(-130)
    end
end

function lt.KeyUp(key)
    keys[key] = false
    if key == "left" or key == "right" then
        body:SetAngularVelocity(0)
    end
end

function lt.Advance()
    local angle = body:GetAngle()
    if keys.up then
        body:ApplyForce(sin(-angle) * 30, cos(-angle) * 30)
    end

    world:Step(lt.secs_per_frame)
end

-- Draw grid lines
local grid = lt.Scene()
for y = bottom, top, grid_gap do
    grid:Insert(lt.Line(left, y, right, y), 0)
end
for x = left, right, grid_gap do
    grid:Insert(lt.Line(x, bottom, x, top), 0)
end
scene:Insert(lt.Tinter(grid, grey()), 0)

local function nearest_grid_point(x, y)
    local nx = math.floor(x / grid_gap + 0.5) * grid_gap;
    local ny = math.floor(y / grid_gap + 0.5) * grid_gap;
    return nx, ny
end

-- Building triangles.
local next_point = 1
local points = {}
local lines = {}
local build_triangle = {}
function build_triangle.MouseDown(button, x, y)
    x, y = nearest_grid_point(x, y)
    points[next_point] = {x = x, y = y}
    if next_point < 3 then
        lines[next_point] = lt.Line(x, y, x, y)
        scene:Insert(lines[next_point], 1)
    end
    if next_point == 3 then
        next_point = 0
        static:AddTriangle(
            points[1].x, points[1].y,
            points[2].x, points[2].y,
            points[3].x, points[3].y);
        points = {}
        for i, l in ipairs(lines) do
            scene:Remove(l)
        end
        lines = {}
    end
    next_point = next_point + 1
end
function build_triangle.MouseUp(button, x, y)
end
function build_triangle.MouseMove(x, y)
    x, y = nearest_grid_point(x, y)
    if next_point > 1 and lines[next_point - 1] then
        lines[next_point - 1]:Set{x2 = x, y2 = y}
    end
end

lt.MouseUp = build_triangle.MouseUp
lt.MouseDown = build_triangle.MouseDown
lt.MouseMove = build_triangle.MouseMove

function lt.Render()
    scene:Draw()
    lt.PushTint(1, 0, 0)
    body:Draw()
    lt.PopTint()
    lt.PushTint(0, 0, 1)
    static:Draw()
    lt.PopTint()
end
