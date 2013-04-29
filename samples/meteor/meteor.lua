import "particles"
import "rock_coords"

math.randomseed(os.time())

local images = lt.LoadImages({
    "bg",
    "meteor",
    "orb",
    "ship",
    "star",
    "survivor1",
    "survivor2",
    {font = "font", glyphs = "ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890"},
})

local samples = lt.LoadSamples({
    "meteor_explode",
    "ship_explode",
    "pickup_survivor",
    "ship_thrust",
})

local world = box2d.World(0, -5)

--world.debug = true
world.scale = 1

local score = 0
local score_text = lt.Text(tostring(score), images.font, "center", "top")
    :Scale(2)
    :Translate(0, lt.config.world_top - 0.1)

local
function update_score()
    score_text.child.child = lt.Text(tostring(score), images.font, "center", "top")
end

local surface_layer = lt.Layer()
surface_layer:Action(function(dt)
    world:Step(dt)
end)

local
function make_surface()
    local l, b, r, t = lt.config.world_left, lt.config.world_bottom, lt.config.world_right, lt.config.world_top
    local surface_body = world:Body({type="static"})

    -- left edge.
    surface_body:Polygon{l - 10, b, l - 10, t, l, t, l, b}.rock = true
    -- right edge.
    surface_body:Polygon{r + 10, b, r + 10, t, r, t, r, b}.rock = true
    -- top edge.
    surface_body:Polygon{l - 10, t, r + 10, t, r + 10, t + 10, l - 10, t + 10}
    -- bottom edge.
    surface_body:Polygon{l - 10, b, r + 10, b, r + 10, b - 10, l - 10, b - 10}.rock = true

    -- rocks
    for _, p in ipairs(rock_polys) do
        surface_body:Polygon(p).rock = true
    end

    surface_layer:Insert(surface_body)
end

make_surface()

local ship
local ship_layer = lt.Layer()
local main_layer = lt.Layer()

local meteor_layer = lt.Layer()

local
function make_meteor()
    local angle = -135 + math.random() * 90
    local speed = 2 + math.min(math.random() * 1.5 * (score * 0.2 + 1), 5)
    local vx = cos(angle) * speed
    local vy = sin(angle) * speed
    local x = math.random() * 6 - 6
    local y = lt.config.world_top + 1

    local meteor_body = world:Body({type="kinematic", x = x, y = y, vx = vx, vy = vy})
    local meteor_fixture = meteor_body:Circle(0.2, 0, 0, {sensor=true})
    meteor_fixture.meteor = true
    meteor_body.child = lt.Layer()
    meteor_body.child:Insert(images.meteor:Scale(0.3))
    meteor_body.child:Insert(meteor_trail(images, angle + 180))
    meteor_body:Action(function(dt)
        local touching = meteor_fixture:Touching()
        for _, f in ipairs(touching) do
            if f.rock then
                local x, y = meteor_body.x, meteor_body.y
                meteor_layer:Remove(meteor_body)
                meteor_body:Destroy()
                local explosion = meteor_explosion(images, angle + 180):Translate(x, y)
                meteor_layer:Insert(explosion)
                meteor_layer:Action(function(dt)
                    if explosion.finished then
                        meteor_layer:Remove(explosion)
                        return true
                    end
                end)
                samples.meteor_explode:Play()
                return true
            elseif f.ship then
                local x, y = meteor_body.x, meteor_body.y
                local explosion = ship_explosion(images):Translate(x, y)
                meteor_layer:Insert(explosion)
                ship:Destroy()
                main_layer:Remove(ship_layer)
                local gameover_text = lt.Text(
                    "GAME OVER\n\nPRESS ENTER TO PLAY AGAIN\n\nOR ESC TO QUIT",
                    images.font, "center", "center"):Scale(1.5)
                gameover_text:KeyUp(function(event)
                    if event.key == "enter" then
                        import "meteor"
                    end
                end)
                main_layer:Insert(gameover_text)
                samples.ship_explode:Play()
            end
        end
    end)
    meteor_layer:Insert(meteor_body)
end

meteor_layer:Action(function(dt)
    make_meteor()
    return (math.random() * 2 + 1) / (score * 0.2 + 1) + 0.1 -- delay
end)

local
function make_ship()
    local x, y = 0, 2
    local w, h = 0.2, 0.6
    local ang_v = 180
    local thrust = 2
    local ship_body = world:Body({type="dynamic", x = x, y = y, angle = 0, damping = 2.8})
    ship_body.child = images.ship:Scale(0.6):Translate(0.05, 0.4)
    local ship_particles = ship_trail(images)
    local ship_fixture = ship_body:Polygon{-w, 0, w, 0, 0, h}
    ship_fixture.ship = true

    local thrust_track = lt.Track()
    thrust_track.loop = true
    thrust_track:Queue(samples.ship_thrust)

    local key = {}
    ship_body:KeyDown(function(event)
        key[event.key] = true
        if event.key == "up" then
            thrust_track:Play()
        end
    end)
    ship_body:KeyUp(function(event)
        key[event.key] = nil
        if event.key == "up" then
            thrust_track:Pause()
        end
    end)

    ship_body:Action(function(dt)
        local angle = ship_body.angle
        if key.left then
            ship_body.angular_velocity = ang_v
        elseif key.right then
            ship_body.angular_velocity = -ang_v
        else
            ship_body.angular_velocity = 0
        end
        if key.up then
            local fx = cos(angle + 90) * thrust
            local fy = sin(angle + 90) * thrust
            ship_body:Force(fx, fy)
            ship_particles.emission_rate = 200
        else
            ship_particles.emission_rate = 0
        end
        ship_particles.angle = angle - 90
        ship_particles.source_position_x = ship_body.x + cos(angle + 90) * 0.15
        ship_particles.source_position_y = ship_body.y + sin(angle + 90) * 0.15
    end)
    ship_layer:Insert(ship_particles)
    ship_layer:Insert(ship_body)
    ship_layer:Insert(thrust_track)
    ship = ship_body
end

make_ship()

local survivors_layer = lt.Layer()
local prev_survivor_pos

local
function make_survivor()
    local w = 0.3
    local h = 0.4
    local survivor_positions = {
        { -5.53125, 0.828125, },
        { -3, -3.109375, },
        { 1.953125, -4.09375, },
        { 5.375, -1.28125, },
    }
    repeat
        pos = math.random(#survivor_positions)
    until pos ~= prev_survivor_pos
    prev_survivor_pos = pos
    local xy = survivor_positions[pos]
    local survivor_body = world:Body({type="static", x = xy[1], y = xy[2]})
    local survivor_fixture = survivor_body:Polygon({-w, -h, w, -h, w, h, -w, h})
    survivor_body:Action(function(dt)
        local touching = survivor_fixture:Touching()
        for _, f in ipairs(touching) do
            if f.ship then
                survivor_body:Destroy()
                survivors_layer:Remove(survivor_body)
                make_survivor()
                score = score + 1
                update_score()
                samples.pickup_survivor:Play()
                return true
            end
        end
    end)
    survivor_body.child = images.survivor1:Scale(0.6)
    local frame = 1
    survivor_body.child:Action(function(dt, node)
        node.child = images["survivor" .. frame]
        frame = frame + 1
        if frame > 2 then
            frame = 1
        end
        return 0.4
    end)
    survivors_layer:Insert(survivor_body)
end

make_survivor()

local bg_layer = lt.Layer()
bg_layer:Insert(images.bg)

main_layer:Insert(bg_layer)
main_layer:Insert(meteor_layer)
main_layer:Insert(surface_layer)
main_layer:Insert(survivors_layer)
main_layer:Insert(ship_layer)
main_layer:Insert(score_text)

lt.root.child = main_layer

-- Used to find world coordinates during development
--main_layer:MouseDown(function(event)
--    log(event.x .. ", " .. event.y .. ",")
--end)

