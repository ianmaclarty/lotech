-- Copyright 2011 Ian MacLarty
local
function cleanup_obj_tween_fields(obj)
    obj._tweens = nil
    obj._tween_actions = nil
    obj._tween_index = nil
    obj._tweenset = nil
end

function lt.RemoveTweens(obj)
    local tweens = obj._tweens
    if tweens then
        tweens[obj._tween_index] = nil
        cleanup_obj_tween_fields(obj)
    end
end

function lt.ClearTweenSet(tweens)
    for index, obj in pairs(tweens) do
        cleanup_obj_tween_fields(obj)
        tweens[index] = nil
    end
end

local lt_advance_native_tween = lt.AdvanceNativeTween

function lt.AdvanceTweens(tweens, dt)
    local actions = {}
    --local count = 0
    --local sample_field = "nothing"
    for obj_index, obj in pairs(tweens) do
        local obj_tweens = obj._tweens
        for field, tween in pairs(obj_tweens) do
            --count = count + 1
            --sample_field = field
            local finished = false
            if type(tween) == "userdata" then
                -- native tween
                finished = lt_advance_native_tween(tween, dt)
            else
                local delay = tween.delay
                if delay > 0 then
                    tween.delay = delay - dt
                else 
                    local t = tween.t
                    if t < 1 then
                        local v0 = tween.v0
                        local v = v0 + (tween.v - v0) * tween.ease(t)
                        tween.t = t + dt / tween.period
                        obj[field] = v
                    else
                        obj[field] = tween.v
                        finished = true
                    end
                end
            end
            if finished then
                local tween_actions = obj._tween_actions
                if tween_actions then
                    local action = tween_actions[field]
                    if action then
                        table.insert(actions, action)
                        tween_actions[field] = nil
                    end
                end
                obj_tweens[field] = nil
            end
        end
        if next(obj_tweens) == nil then
            -- No more tweens on object, so remove from tween set and clean up.
            cleanup_obj_tween_fields(obj)
            tweens[obj_index] = nil
        end
    end
    for _, action in ipairs(actions) do
        action()
    end
    --log("tween count = " .. count .. " field = " .. sample_field)
end

local lt_get = lt.GetObjectField
-- Finds the owner of the field in the obj or its
-- descendents.  Returns the owner or nil if the field
-- doesn't exist.  Also returns true if the field is a
-- C field (and therefore a candidate for fast tweening).
local
function find_field_owner(obj, field)
    local value = obj[field]
    if value then
        return obj
    end
    local child = obj.child
    if child then
        return find_field_owner(child, field)
    else
        return nil
    end
end

local make_native_tween = lt.MakeNativeTween
local ease_func_table

function lt.AddTween(tweens, table, field, to, secs, delay, ease, action, called_from_tween_method)
    --local obj = find_field_owner(table, field)
    local obj = table
    if not obj[field] then
        local level = called_from_tween_method and 3 or 2
        error("No such field: " .. field, level)
    end
    local tween
    --[[
    if type(obj) == "userdata" and obj.is then
        tween = make_native_tween(obj, field, delay, to, secs, ease)
    end
    ]]
    if not tween then
        if ease == nil then
            ease = lt.LinearEase
        elseif type(ease) == "string" then
            local func = ease_func_table[ease]
            if not func then
                local level = called_from_tween_method and 3 or 2
                error("Unsupported easing function: " .. ease, level)
            end
            ease = func
        end
        tween = {
            v0 = obj[field],
            v = to,
            t = 0,
            period = secs,
            delay = delay,
            ease = ease,
        }
    end
    local obj_tweens = obj._tweens
    if not obj_tweens then
        obj_tweens = {}
        obj._tweens = obj_tweens
    end
    obj_tweens[field] = tween
    local obj_actions = obj._tween_actions
    if action then
        if not obj_actions then
            obj_actions = {}
            obj._tween_actions = obj_actions
        end
        obj_actions[field] = action
    elseif obj_actions then
        -- Remove any action that might have been there before
        obj_actions[field] = nil
    end
    local obj_tweenset = obj._tweenset
    obj._tweenset = obj_tweenset
    local obj_index = obj._tween_index
    if not obj_index then
        obj_index = #tweens + 1
        obj._tween_index = obj_index
    end
    tweens[obj_index] = obj
end

-------------------------------------------------------------

function lt.LinearEase(t)
    return t
end

function lt.EaseIn(t)
    return math.pow(t, 3)
end

function lt.EaseOut(t)
    return math.pow(t - 1, 3) + 1
end

function lt.EaseInOut(t)
    t = t * 2
    if t < 1 then
        return math.pow(t, 3) / 2
    end
    t = t - 2
    return (math.pow(t, 3) + 2) / 2
end

function lt.BackInEase(t)
    local s = 1.70158
    return t * t * ((s + 1) * t - s)
end

function lt.BackOutEase(t)
    t = t - 1
    local s = 1.70158
    return t * t * ((s + 1) * t + s) + 1
end

function lt.ElasticEase(t)
    if t == 0 or t == 1 then
        return t
    end
    local p = 0.3
    local s = p / 4
    return math.pow(2, -10 * t) * math.sin((t - s) * (2 * math.pi) / p) + 1
end

function lt.BounceEase(t)
    local s = 7.5625
    local p = 2.75
    local l
    if t < 1 / p then
        l = s * t * t
    else
        if t < 2 / p then
            t = t - 1.5 / p
            l = s * t * t + 0.75
        else
            if t < 2.5 / p then
                t = t - 2.25 / p
                l = s * t * t + 0.9375
            else
                t = t - 2.625 / p
                l = s * t * t + 0.984375
            end
        end
    end
    return l
end

function lt.CubicBezierEase(p1x, p1y, p2x, p2y)
    return function(t)
        local cx = 3 * p1x
        local bx = 3 * (p2x - p1x) - cx
        local ax = 1 - cx - bx
        local cy = 3 * p1y
        local by = 3 * (p2y - p1y) - cy
        local ay = 1 - cy - by
        local function sampleCurveX(t)
            return ((ax * t + bx) * t + cx) * t
        end
        local function solveCurveX(x, epsilon)
            local t0, t1, t2, x2, d2, i
            t2 = x
            for i = 0, 7 do
                x2 = sampleCurveX(t2) - x
                if math.abs(x2) < epsilon then
                    return t2
                end
                d2 = (3 * ax * t2 + 2 * bx) * t2 + cx
                if math.abs(d2) < 1e-6 then
                    break
                end
                t2 = t2 - x2 / d2
            end
            t0 = 0
            t1 = 1
            t2 = x
            if t2 < t0 then
                return t0
            end
            if t2 > t1 then
                return t1
            end
            while t0 < t1 do
                x2 = sampleCurveX(t2)
                if math.abs(x2 - x) < epsilon then
                    return t2
                end
                if x > x2 then
                    t0 = t2
                else
                    t1 = t2
                end
                t2 = (t1 - t0) / 2 + t0
            end
            return t2
        end
        local t2 = solveCurveX(t, 0.001)
        return ((ay * t2 + by) * t2 + cy) * t2
    end
end

function lt.AccelEase(t)
    return t * t
end

function lt.DeccelEase(t)
    local t1 = 1 - t
    return 1 - (t1 * t1)
end

function lt.ZoomInEase(t)
    local s = 0.05
    return (1 / (1 + s - t) - 1) * s
end

function lt.ZoomOutEase(t)
    local s = 0.05
    return 1 + s - s / (s + t)
end

function lt.RevolveEase(t)
    return (math.sin(math.pi * (t - 0.5)) + 1) * 0.5
end

ease_func_table = {
    bounce = lt.BounceEase,
    elastic = lt.ElasticEase,
    backin = lt.BackInEase,
    backout = lt.BackOutEase,
    ["in"] = lt.EaseIn,
    out = lt.EaseOut,
    linear = lt.LinearEase,
    accel = lt.AccelEase,
    decel = lt.DeccelEase,
    zoomin = lt.ZoomInEase,
    zoomout = lt.ZoomOutEase,
    revolve = lt.RevolveEase,
}

-------------------------------------------------------------
local tweens_mt = {
    __mode = "v",
    __index = {
        Add = function(tweenset, obj, spec) lt.Tween(obj, spec, tweenset) end,
        Advance = lt.AdvanceTweens,
        Clear = lt.ClearTweenSet,
    },
}

function lt.TweenSet()
    local tweens = {}
    setmetatable(tweens, tweens_mt)
    return tweens
end

-------------------------------------------------------------

local global_tweens = lt.TweenSet()

-- node:Tween{x = 5, y = 6, time = 2.5, easing = "linear", tweens = my_tween_set, action = function() log("done!") end}
function lt.Tween(node, tween_info, tweenset)
    local tweens = tweenset or global_tweens
    local fields = {}
    local time = 0
    local delay = 0
    local easing = nil
    local action = nil
    local reverse = false
    for field, value in pairs(tween_info) do
        if field == "time" then
            time = value
        elseif field == "easing" then
            easing = value
        elseif field == "action" then
            action = value
        elseif field == "tweens" then
            tweens = value
        elseif field == "delay" then
            delay = value
        elseif field == "reverse" then
            reverse = value
        else
            fields[field] = value
        end
    end
    for field, value in pairs(fields) do
        if reverse then
            node[field], value = value, node[field]
        end
        lt.AddTween(tweens, node, field, value, time, delay, easing, action, true)
        action = nil -- only attach action to one field
    end
    return node
end

function lt.CancelTween(obj, field)
    local owner = find_field_owner(obj, field)
    local obj_tweens = owner._tweens
    if obj_tweens then
        obj_tweens[field] = nil
        if next(obj_tweens) == nil then
            local tweenset = owner._tweenset
            local tween_index = owner._tween_index
            tweenset[tween_index] = nil
            owner._tweens = nil
            owner._tween_actions = nil
            owner._tween_index = nil
            owner._tweenset = nil
        else
            local obj_actions = owner._tween_actions
            if obj_actions then
                obj_actions[field] = nil
            end
        end
    end
end

function lt.AdvanceGlobalTweens(dt)
    lt.AdvanceTweens(global_tweens, dt or lt.secs_per_frame)
end

function lt.ClearGlobalTweens(dt)
    global_tweens:Clear();
end
