local tweens_mt = {__mode = "k"}

-------------------------------------------------------------

function lt.TweenSet()
    local tweens = {}
    setmetatable(tweens, tweens_mt) -- make tweens' keys weak references
    return tweens
end

function lt.AdvanceTweens(tweens, dt)
    local actions = {}
    for table, fields in pairs(tweens) do
        for field, tween in pairs(fields) do
            local delay = tween.delay
            if delay > 0 then
                tween.delay = delay - dt
            else 
                local t = tween.t
                if t < 1 then
                    local v0 = tween.v0
                    local v = v0 + (tween.v - v0) * tween.ease(t)
                    tween.t = t + dt / tween.period
                    table[field] = v
                else
                    table[field] = tween.v
                    if tween.done then
                        actions[tween.done] = true
                    end
                    fields[field] = nil
                    if next(fields) == nil then
                        tweens[table] = nil
                    end
                end
            end
        end
    end
    for action in next, actions do
        action()
    end
end

function lt.AddTween(tweens, table, field, to, secs, delay, ease, onDone)
    if ease == nil then
        ease = lt.LinearEase
    end
    local tween = {
        v0 = table[field],
        v = to,
        t = 0,
        period = secs,
        delay = delay,
        ease = ease,
        done = onDone
    }
    if tweens[table] == nil then
        tweens[table] = {[field] = tween}
    else
        tweens[table][field] = tween
    end
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

-------------------------------------------------------------

local global_tweens = lt.TweenSet()

-- node:Tween{x = 5, y = 6, time = 2.5, easing = "linear", tweens = my_tween_set, action = function() log("done!") end}
function lt.Tween(node, tween_info)
    local tweens = global_tweens
    local fields = {}
    local time = 0
    local delay = 0
    local easing = lt.LinearEase
    local action = nil
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
        else
            fields[field] = value
        end
    end
    for field, value in pairs(fields) do
        if time == 0 and delay == 0 then
            -- If time == 0 don't bother adding a new tween
            if tweens[node] then
                tweens[node][field] = nil -- delete existing tween
            end
            node[field] = value
            if action then
                action()
            end
        else
            lt.AddTween(tweens, node, field, value, time, delay, easing, action)
        end
        action = nil -- only attach action to one field
    end
    return node
end

function lt.AdvanceGlobalTweens()
    lt.AdvanceTweens(global_tweens, lt.secs_per_frame)
end
