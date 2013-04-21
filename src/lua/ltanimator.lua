-- Copyright (C) 2010-2013 Ian MacLarty. See Copyright Notice in ../lt.h
local
function animator_advance(animator, dt)
    for i = 1, animator.num_threads do
        local thread = animator.threads[i]
        if thread then
            local accum = animator.accums[i]
            local done = false
            accum = accum - dt
            while accum < 0 and not done do
                local t = thread(animator.tweens)
                log(tostring(t))
                if not t then
                    done = true
                    animator.threads[i] = nil
                else
                    accum = accum + t
                end
            end
            animator.accums[i] = accum
        end
    end
    animator.tweens:Advance(dt)
end

local lt_add_tween = lt.AddTween

local
function add_tween(tweens, obj, field, val, time, easing)
    while not lt_add_tween(tweens, obj, field, val, time, easing) and obj do
        obj = obj.child
    end
    if not obj then
        error("Cannot tween field: " .. field, 3)
    end
end

function lt.AddTweens(tweens, obj, fields)
    local time = fields.time
    if not time then
        time = 1
    end
    local easing = fields.easing
    if not easing then
        easing = "linear"
    end
    for field, val in pairs(fields) do
        if field ~= "time" and field ~= "easing" then
            add_tween(tweens, obj, field, val, time, easing)
        end
    end
end

function lt.Animator(...)
    local n = select("#", ...)
    local tweens = lt.TweenSet()
    local threads = {}
    local accums = {}
    for i = 1, n do
        local f = select(i, ...)
        local thread = coroutine.wrap(f)
        accums[i] = thread(tweens)
        threads[i] = thread
    end
    local animator = {
        num_threads = n,
        tweens = tweens,
        threads = threads,
        accums = accums, -- amount of time to wait before resuming each thread
        Advance = animator_advance,
    }
    return animator
end

function lt.Wait(t)
    coroutine.yield(t)
end
