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

local layer = lt.Layer()

-- Draw grid lines
local grid = lt.Layer()
for y = bottom, top, grid_gap do
    grid:Insert(lt.Line(left, y, right, y), 0)
end
for x = left, right, grid_gap do
    grid:Insert(lt.Line(x, bottom, x, top), 0)
end
grid = lt.Tint(grid, grey())

local function nearest_grid_point(x, y)
    local nx = math.floor(x / grid_gap + 0.5) * grid_gap
    local ny = math.floor(y / grid_gap + 0.5) * grid_gap
    return nx, ny
end

local world, static

local mouse_pos = {x = 0, y = 0}
local paused = true
local keys = {}
local ship = {}
local current_mode = "draw"
local modekeys = {}
modekeys.D = "draw"
modekeys.S = "select"
local mode = {}

local level = {
    start = {
        x = 0,
        y = 0,
    },
    triangles = {},
}

local colors = {
    red         = {r = 1, g = 0, b = 0},
    blue        = {r = 0, g = 0, b = 1},
    green       = {r = 0, g = 1, b = 0},
    yellow      = {r = 1, g = 1, b = 0},
    magenta     = {r = 1, g = 0, b = 1},
    cyan        = {r = 0, g = 1, b = 1},
}
local color_keys = {}
local i = 1
for key, rgb in pairs(colors) do
    color_keys[i] = key
    i = i + 1
end
local color = "red"

local function add_triangle(t, p)
    local c = colors[t.color]
    local node = lt.Tint(lt.Triangle(
        t.x1, t.y1, t.x2, t.y2, t.x3, t.y3), c.r, c.g, c.b, 0.7)
    layer:Insert(node)
    local fixture = static:AddTriangle(
        t.x1, t.y1, t.x2, t.y2, t.x3, t.y3)
    fixture.triangle_index = p
    fixture.layer_node = node
end

local function init_level()
    layer = lt.Layer()
    layer:Insert(grid, 0)
    world = lt.World()
    world:SetGravity(0, -10)
    static = world:AddStaticBody()
    ship.init()
    mode.select.selected = nil
    for p, t in pairs(level.triangles) do
        add_triangle(t, p)
    end
end

---------------------------------------------------------------------------------
-- Input

function lt.KeyDown(key)
    keys[key] = true
    if key == "P" then
        paused = not paused
    elseif key == "R" then
        init_level()
    elseif key == "del" then
        mode.select.delete_selected()
    elseif modekeys[key] then
        current_mode = modekeys[key]
    elseif tonumber(key) then
        local n = tonumber(key)
        if n >= 1 and n <= #color_keys then
            color = color_keys[n]
            print(color)
        end
    elseif key == "W" then
        lt.Save("level.data", level)
    elseif key == "L" then
        level = lt.Load("level.data")
        init_level()
    elseif key == "B" then
        level.start = clone(mouse_pos)
        init_level()
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
    mouse_pos.x = x
    mouse_pos.y = y
end

---------------------------------------------------------------------------------
-- Ship

function ship.init()
    ship.body = world:AddDynamicBody(level.start.x, level.start.y, 0)
    ship.body:AddTriangle(-0.5, -0.8, 0.5, -0.8, 0, 0.8, 1)
    layer:Insert(lt.Tint(ship.body, 1, 0, 0), 1)
end

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
        layer:Insert(lines[next_point], 1)
    end
    if next_point == 3 then
        next_point = 0
        local pos = #level.triangles + 1
        local triangle = {
            x1 = points[1].x, y1 = points[1].y,
            x2 = points[2].x, y2 = points[2].y,
            x3 = points[3].x, y3 = points[3].y,
            color = color
        }
        table.insert(level.triangles, pos, triangle)
        add_triangle(triangle, pos)
        points = {}
        for i, l in ipairs(lines) do
            layer:Remove(l)
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
        local c = colors[level.triangles[selected.triangle_index].color]
        selected.layer_node:Set{r = c.r, g = c.g, b = c.b, a = 0.7}
        selected = nil
    end
    local fixtures = world:QueryBox(x - grid_gap, y - grid_gap, x + grid_gap, y + grid_gap)
    for _, fixture in ipairs(fixtures) do
        if fixture:ContainsPoint(x, y) and fixture:GetBody() ~= ship.body then
            selected = fixture
            selected.layer_node:Set{r = 1, b = 1, g = 1, a = 0.8}
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
        layer:Remove(selected.layer_node)
        level.triangles[selected.triangle_index] = nil
        selected:Destroy()
        selected = nil
    end
end

setfenv(mode.select.MouseDown, mode.select)
setfenv(mode.select.MouseUp, mode.select)
setfenv(mode.select.MouseMove, mode.select)
setfenv(mode.select.delete_selected, mode.select)
setmetatable(mode.select, mode.select)

---------------------------------------------------------------------------------

init_level()

function lt.Render()
    layer:Draw()
end
