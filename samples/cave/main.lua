images = lt.LoadImages({
    "wall_edge",
    "wall_inner"
}, "nearest", "nearest")

samples = lt.LoadSamples({
    "laser"
})

import "bg"

math.randomseed(os.time())

lt.FixGlobals()
--------------------------------------

local main_layer = lt.Layer()
local rt = main_layer
    :RenderTarget(240, 160, 0, 0, 480, 320, 0, 0, 480, 320, "nearest", "nearest")
local top = rt:BlendMode("off")
lt.root:Insert(top)

local fire_laser

-- Input
local key_down = {}
main_layer:KeyDown(function(event)
    key_down[event.key] = true
    if event.key == "X" then
        fire_laser()
    end
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
    local scroll_speed = 140
    local top_fixture, bottom_fixture
    local top_verts, bottom_verts = {}, {}
    local w = 1000
    local w2 = w/2
    local dx = 80
    local
    function regen_walls()
        local x = top_verts[#top_verts - 1] or -dx
        while x < w do
            local top_y = math.random() * 100 + 200
            local bottom_y = top_y - (math.random() * 100 + 100)
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
    local walls = make_cave_wall_bg(top_verts, bottom_verts):Translate(0, 0)
    local curr_x = 0
    cave_wall_body:Action(function(dt, b)
        curr_x = curr_x - scroll_speed * dt
        b.x = curr_x
        walls.x = math.floor(curr_x * 0.5) * 2
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
            walls = make_cave_wall_bg(top_verts, bottom_verts):Translate(curr_x, 0)
            main_layer:InsertBelow(cave_wall_body, walls)
        end
    end)
    main_layer:Insert(walls)
end

main_layer:Insert(cave_wall_body)

-- Ship
local ship_body = world:Body{type="kinematic", x = 10, y = ship_start_y}
local ship_fixture = ship_body:Polygon{-10, -10, -10, 10, 10, 0}

local
function die()
    main_layer:Remove(ship_body)
    ship_body:Destroy()

    rt:Tween{pwidth = 2, pheight = 1, time = 2, easing = "zoomout", action = function()
        lt.root:Remove(top)
        import "main"
    end}
end

local ship_speed = 180
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
    if b.x < 10 then
        b.x = 10
    elseif b.x > 400 then
        b.x = 400
    end
    for i, fixture in ipairs(ship_fixture:Touching()) do
        if fixture.cave_wall then
            die()
            return true
        end
    end
end)
main_layer:Insert(ship_body)

function fire_laser()
    if not ship_body then
        return
    end
    local speed = 400
    local x = ship_body.x
    local y = ship_body.y
    local laser_body = world:Body({type="kinematic", x = x, y = y})
    local h = 5
    local w = 10
    local laser_fixture = laser_body:Polygon{-w, -h, -w, h, w, h, w, -h}
    laser_body:Action(function(dt, b)
        x = x + speed * dt
        if x > 500 then
            b:Destroy()
            main_layer:Remove(b)
        end
        b.x = x
        for _, f in ipairs(laser_fixture:Touching()) do
            if f.cave_wall or f.enemy then
                b:Destroy()
                main_layer:Remove(b)
            end
            if f.enemy then
                f.body:Destroy()
                main_layer:Remove(f.body)
            end
        end
    end)
    main_layer:Insert(laser_body)
    samples.laser:Play()
end

-- Enemies

local
function make_enemy()
    local speed = 70
    local x = 500
    local y = 200
    local enemy_body = world:Body({type="kinematic", x = x, y = y})
    local enemy_fixture = enemy_body:Circle(10, 0, 0)
    enemy_fixture.enemy = true
    main_layer:Insert(enemy_body)
    enemy_body:Action(function(dt, b)
        x = x - speed * dt
        if x < -20 then
            b:Destroy()
            main_layer:Remove(b)
            return true
        end
        b.x = x
    end)
end

main_layer:Action(coroutine.wrap(function(dt)
    while true do
        coroutine.yield(math.random() * 2 + 2)
        make_enemy()
    end
end))
