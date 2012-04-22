-- Copyright 2011 Ian MacLarty
function clone(table)
    local copy = {}
    for k, v in pairs(table) do
        copy[k] = v
    end
    return copy
end

function lt.ClearFuncs()
    lt.Advance = nil
    lt.Render = nil
    lt.PointerDown = nil
    lt.PointerUp = nil
    lt.PointerMove = nil
    lt.KeyUp = nil
    lt.KeyDown = nil
end

function lt.AddSpriteFiles(tab, name, num_frames)
    for i = 1, num_frames do
        table.insert(tab, string.format("%s_%04d", name, i))
    end
    return tab
end

function lt.MatchFields(tab, pattern)
    local fields = {}
    local values = {}
    for field, value in pairs(tab) do
        if type(field) == "string" then
            if string.match(field, pattern) then
                table.insert(fields, field)
            end
        end
    end
    table.sort(fields)
    for i, field in ipairs(fields) do
        values[i] = tab[field]
    end
    return values
end

function lt.Store(key, val)
    local t = type(val)
    if t == "string" or t == "number" or t == "boolean" or t == "nil" then
        local state = lt.state
        state[key] = val
    else
        log("tried to store a value of type " .. t .. " for key " .. key)
    end
end

function lt.Retrieve(key)
    local state = lt.state
    if state then
        return state[key]
    else
        return nil
    end
end

function lt.LogGlobals()
    log("Globals:")
    for key, _ in pairs(_G) do
        log(key)
    end
end

-- Prevents creation of any new globals.
function lt.FixGlobals()
    local mt = {
        __newindex = function(table, key, value)
            if rawget(table, key) ~= nil then
                rawset(table, key, value)
            else
                error("Attempt to create new global: " .. key, 2)
            end
        end
    }
    setmetatable(_G, mt)
end

function lt.SceneNodeContainsPoint(node, x, y, buf)
    buf = buf or 0
    local t, b, l, r = node.top, node.bottom, node.left, node.right
    return x <= r + buf and x >= l - buf and y <= t + buf and y >= b - buf
end
