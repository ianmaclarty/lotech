dofile("../src/ltlua/lt.lua")

---------------------------------------------------------------------------------

local function grey()
    return 0.1, 0.1, 0.1
end

---------------------------------------------------------------------------------
-- Setup

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

local scene = lt.Scene()

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

local world = lt.World()
world:SetGravity(0, -10)

local static = world:StaticBody()
static:AddRect(left, bottom, right, bottom + 0.5)
static:AddRect(left, top - 0.5, right, top)
static:AddRect(left, bottom, left + 0.5, top)
static:AddRect(right - 0.5, bottom, right, top)
scene:Insert(lt.Tinter(static, 0.5, 0.5, 0.5, 0.4), 1)

local paused = false
local keys = {}
local ship = {}
local current_mode = "draw"
local modekeys = {}
modekeys.D = "draw"
modekeys.S = "select"
local mode = {}

---------------------------------------------------------------------------------
-- Input

function lt.KeyDown(key)
    keys[key] = true
    if key == "P" then
        paused = not paused
    elseif key == "R" then
        ship.init()
    elseif key == "backspace" then
        mode.select.delete_selected()
    elseif modekeys[key] then
        current_mode = modekeys[key]
    else
        ship.KeyDown(key)
    end
end

function lt.KeyUp(key)
    keys[key] = false
    ship.KeyUp(key)
end

function lt.Advance()
    if not paused then
        world:Step(lt.secs_per_frame)
    end
    ship.Advance()
end

function lt.MouseUp(button, x, y)
    mode[current_mode].MouseUp(button, x, y)
end
function lt.MouseDown(button, x, y)
    mode[current_mode].MouseDown(button, x, y)
end
function lt.MouseMove(x, y)
    mode[current_mode].MouseMove(x, y)
end

---------------------------------------------------------------------------------
-- Ship

function ship.init()
    if ship.body then
        ship.body:Destroy()
        scene:Remove(ship.body)
    end
    ship.body = world:DynamicBody(0, 0, 0)
    ship.body:AddTriangle(-0.5, -0.8, 0.5, -0.8, 0, 0.8, 1)
    scene:Insert(lt.Tinter(ship.body, 1, 0, 0), 1)
end

ship.init()

function ship.KeyDown(key)
    if key == "left" then
        ship.body:SetAngularVelocity(130)
    elseif key == "right" then
        ship.body:SetAngularVelocity(-130)
    end
end
function ship.KeyUp(key)
    if key == "left" or key == "right" then
        ship.body:SetAngularVelocity(0)
    end
end
function ship.Advance()
    local angle = ship.body:GetAngle()
    if keys.up then
        ship.body:ApplyForce(sin(-angle) * 30, cos(-angle) * 30)
    end
end

---------------------------------------------------------------------------------
-- Draw mode.

mode.draw = {
    next_point = 1,
    points = {},
    lines = {},
    __index = _G
}

function mode.draw.MouseDown(button, x, y)
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

function mode.draw.MouseUp(button, x, y)
end

function mode.draw.MouseMove(x, y)
    x, y = nearest_grid_point(x, y)
    if next_point > 1 and lines[next_point - 1] then
        lines[next_point - 1]:Set{x2 = x, y2 = y}
    end
end

setfenv(mode.draw.MouseDown, mode.draw)
setfenv(mode.draw.MouseUp, mode.draw)
setfenv(mode.draw.MouseMove, mode.draw)
setmetatable(mode.draw, mode.draw)

---------------------------------------------------------------------------------
-- Select mode.

mode.select = {
    selected = nil,
    __index = _G
}

function mode.select.MouseDown(button, x, y)
    if selected then
        scene:Remove(selected.prop)
        selected = nil
    end
    local fixtures = world:QueryBox(x - grid_gap, y - grid_gap, x + grid_gap, y + grid_gap)
    for _, fixture in ipairs(fixtures) do
        if fixture:ContainsPoint(x, y) and fixture:GetBody() ~= ship.body then
            selected = {
                fixture = fixture,
                prop = lt.Tinter(fixture, 1, 1, 1, 0.4)
            }
            scene:Insert(selected.prop, 2)
            break
        end
    end
end

function mode.select.MouseUp(button, x, y)
end

function mode.select.MouseMove(x, y)
end

function mode.select.delete_selected()
    if selected then
        scene:Remove(selected.prop)
        selected.fixture:Destroy()
        selected = nil
    end
end

setfenv(mode.select.MouseDown, mode.select)
setfenv(mode.select.MouseUp, mode.select)
setfenv(mode.select.MouseMove, mode.select)
setfenv(mode.select.delete_selected, mode.select)
setmetatable(mode.select, mode.select)

---------------------------------------------------------------------------------

function lt.Render()
    scene:Draw()
end
