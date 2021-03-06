-- Copyright (C) 2010-2013 Ian MacLarty. See Copyright Notice in ../lt.h
function lt.Tween(node, tween_info)
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
        elseif field == "delay" then
            delay = value
        --elseif field == "reverse" then
        --    reverse = value
        else
            fields[field] = value
        end
    end
    for field, value in pairs(fields) do
        --if reverse then
        --    local tmp = node[field]
        --    node[field] = value
        --    value = tmp
        --end
        lt.AddTween(node, field, value, time, delay, easing, action)
        action = nil -- only attach action to one field
    end
    return node
end

function lt.CancelTween(obj, field)
    error("NYI")
end

function lt.RemoveTweens(obj)
    error("NYI")
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
