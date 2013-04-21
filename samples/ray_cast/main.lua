local world = box2d.World()
world.debug = true

local layer = lt.Layer()

local static = world:Body({type = "static"})
static.blah = true
static:Polygon({-4, -2, 4, -2, 4, -3, -4, -3}).color = {1, 0, 0}
static:Polygon({-4, -2, -4, 3, -3, 3, -3, -2}).color = {0, 1, 0}
static:Polygon({4, -2, 4, 3, 3, 3, 3, -2}).color = {0, 0, 1}

local rect = lt.Rect(-0.1, -0.1, 0.1, 0.1):Tint(1, 1, 1)
local beam = lt.Rect(0, 0, 100, 0.01):Rotate(0)

layer:Insert(static)
layer:Insert(rect)
layer:Insert(beam)


local angle = 0
layer:Action(function(dt)
    local x = cos(angle) * 100
    local y = sin(angle) * 100
    local res = world:RayCast(0, 0, x, y)
    if #res > 0 and res[1].fixture.body.blah then
        rect.x1 = -0.1 + res[1].x
        rect.y1 = -0.1 + res[1].y
        rect.x2 = 0.1 + res[1].x
        rect.y2 = 0.1 + res[1].y
        rect.red = res[1].fixture.color[1]
        rect.green = res[1].fixture.color[2]
        rect.blue = res[1].fixture.color[3]
        beam.x2 = 100 * res[1].fraction
    else
        rect.x1 = -0.1
        rect.y1 = -0.1
        rect.x2 = 0.1
        rect.y2 = 0.1
        beam.x2 = 100
    end
    angle = angle + dt * 60
    beam.angle = angle

    static.x = sin(angle) * 2
    static.angle = angle * 0.1
end)

lt.root:Insert(layer)
