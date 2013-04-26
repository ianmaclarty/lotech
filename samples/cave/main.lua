images = lt.LoadImages({
    "wall_edge",
    "wall_inner"
}, "nearest", "nearest")

import "bg"

math.randomseed(os.time())

lt.FixGlobals()
--------------------------------------

local main_layer = lt.Layer()
lt.root:Insert(main_layer)

-- Input
local key_down = {}
main_layer:KeyDown(function(event)
    key_down[event.key] = true
end)
main_layer:KeyUp(function(event)
    key_down[event.key] = false
end)

-- World
local wscale = 100
local world = box2d.World()
world.scale = wscale
world.debug = true

-- Cave wall
local cave_wall_body = world:Body{type="kinematic"}
local ship_start_y
do
    local scroll_speed = 1.4
    local top_fixture, bottom_fixture
    local top_verts, bottom_verts = {}, {}
    local w = 10
    local w2 = w/2
    local dx = 0.8
    local
    function regen_walls()
        local x = top_verts[#top_verts - 1] or -dx
        while x < w do
            local top_y = math.random() * 1 + 2
            local bottom_y = top_y - (math.random() * 1 + 1)
            x = x + dx
            table.append(top_verts, {x, top_y})
            table.append(bottom_verts, {x, bottom_y})
        end
        top_fixture = cave_wall_body:Chain(top_verts)
        bottom_fixture = cave_wall_body:Chain(bottom_verts)
        top_fixture.cave_wall = true
        bottom_fixture.cave_wall = true
    end
    regen_walls()
    ship_start_y = (top_verts[2] - bottom_verts[2]) / 2 + bottom_verts[2]
    local walls = make_cave_wall_bg(top_verts, bottom_verts, wscale):Translate(0, 0)
    local curr_x = 0
    cave_wall_body:Action(function(dt, b)
        curr_x = curr_x - scroll_speed * dt
        b.x = curr_x
        walls.x = curr_x * wscale
        if curr_x < -w2 then
            top_fixture:Destroy()
            bottom_fixture:Destroy()
            curr_x = curr_x + w2
            b.x = curr_x
            local new_top_verts, new_bottom_verts = {}, {}
            for i = 1, #top_verts, 2 do
                local x = top_verts[i]
                local next_x = top_verts[i + 2]
                if not next_x or next_x >= w2 then
                    local top_y = top_verts[i + 1]
                    local bottom_y = bottom_verts[i + 1]
                    x = x - w2
                    table.append(new_top_verts, {x, top_y})
                    table.append(new_bottom_verts, {x, bottom_y})
                end
            end
            top_verts = new_top_verts
            bottom_verts = new_bottom_verts
            regen_walls()
            main_layer:Remove(walls)
            walls = make_cave_wall_bg(top_verts, bottom_verts, wscale):Translate(curr_x, 0)
            main_layer:InsertBelow(cave_wall_body, walls)
        end
    end)
    main_layer:Insert(walls)
end

main_layer:Insert(cave_wall_body)

-- Ship
local ship_body = world:Body{type="kinematic", x = 0.1, y = ship_start_y}
local ship_fixture = ship_body:Polygon{-0.1, -0.1, -0.1, 0.1, 0.1, 0}

local
function die()
    main_layer:Remove(ship_body)
    ship_body:Destroy()
    lt.root:Remove(main_layer)
    import "gameover"
end

local ship_speed = 1.8
ship_body:Action(function(dt, b)
    if key_down["up"] then
        b.y = b.y + ship_speed * dt
    end
    if key_down["down"] then
        b.y = b.y - ship_speed * dt
    end
    if key_down["left"] then
        b.x = b.x - ship_speed * dt
    end
    if key_down["right"] then
        b.x = b.x + ship_speed * dt
    end
    if b.x < 0.1 then
        b.x = 0.1
    elseif b.x > 4 then
        b.x = 4
    end
    for i, fixture in ipairs(ship_fixture:Touching()) do
        if fixture.cave_wall then
            die()
            return true
        end
    end
end)

main_layer:Insert(ship_body)
