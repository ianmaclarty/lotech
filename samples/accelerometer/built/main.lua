local base = lt.Rect(-1, -0.9, 1, -1):Tint(1, 1, 0)
local bg = lt.Rect(-1, -1, 1, 1):Tint(1, 0, 0)
local rect = lt.Rect(-0.1, -0.1, 0.1, 0.1):Scale(1):Translate(0, 0):Tint(0, 1, 1)

local layer = lt.Layer()
layer:Insert(bg)
layer:Insert(base)
layer:Insert(rect)

lt.root.child = layer

local window_size = 5
local window = {}
rect:Action(function(dt)
    local x, y, z = lt.SampleAccelerometer()
    table.insert(window, {x, y, z})
    while #window > window_size do
        table.remove(window, 1)
    end
    x, y, z = 0, 0, 0
    for _, sample in ipairs(window) do
        x = x + sample[1]
        y = y + sample[2]
        z = z + sample[3]
    end
    x = x / #window
    y = y / #window
    z = z / #window
    rect.x = x
    rect.y = y
    rect.scale = -z * 2 + 1
end)
