-- Copyright (C) 2010-2013 Ian MacLarty. See Copyright Notice in ../lt.h
function table.clone(t)
    local copy = {}
    for k, v in pairs(t) do
        copy[k] = v
    end
    return copy
end

function table.search(t, elem)
    for i = 1, #t do
        if t[i] == elem then
            return i
        end
    end
    return nil
end

function table.append(arr1, arr2)
    local i = #arr1 + 1
    for _, v in ipairs(arr2) do
        arr1[i] = v
        i = i + 1
    end
end

local
function table_tostring(t, indent)
    local tp = type(t)
    if tp == "table" then
        indent = indent or 0
        local tab = "    "
        local prefix = string.rep(tab, indent)
        local str = "{\n"
        for key, value in pairs(t) do
            local keystr
            if type(key) == "string" then
                keystr = key
            else
                keystr = "[" .. tostring(key) .. "]"
            end
            str = str .. prefix .. tab .. keystr .. " = " .. table_tostring(value, indent + 1) .. ",\n"
        end
        str = str .. prefix .. "}"
        return str
    elseif tp == "string" then
        return '"' .. t:gsub("\"", "\\\"") .. '"'
    else
        return tostring(t)
    end
end

table.tostring = table_tostring

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


lt.config = {
    design_width = 960,
    design_height = 640,
    start_script = "main",
    world_top = 1,
    world_bottom = -1,
    world_left = -1.5,
    world_right = 1.5,
    vsync = true,
    envelope = false,
    fixed_update_time = 1/60,
    fullscreen = false,
    show_mouse_cursor = true,
}

function lt._Setup() 
    lt.config.world_width = lt.config.world_right - lt.config.world_left
    lt.config.world_height = lt.config.world_top - lt.config.world_bottom
    if not lt.config.short_name then
        error("lt.config.short_name not set.  Please set it in config.lua.")
    end
    lt.SetAppShortName(lt.config.short_name)
    lt.SetDesignScreenSize(lt.config.design_width, lt.config.design_height)
    lt.config.orientation = lt.config.design_width > lt.config.design_height and "landscape" or "portrait"
    lt.SetOrientation(lt.config.orientation)

    lt.SetFullScreen(lt.config.fullscreen)
    lt.SetShowMouseCursor(lt.config.show_mouse_cursor)
    lt.SetViewPort(lt.config.world_left, lt.config.world_bottom, lt.config.world_right, lt.config.world_top)
    lt.SetStartScript(lt.config.start_script)

    lt.SetRefreshParams(lt.config.vsync, lt.config.fixed_update_time, lt.config.envelope);
end
