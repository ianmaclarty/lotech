local
function thread_func(f, tweens)
    local cmd
    while true do
        repeat
            cmd = coroutine.yield(0)
        until cmd == "continue"
        local success, res = pcall(f, tweens)
        if success then
            repeat
                cmd = coroutine.yield("finished")
            until cmd == "restart"
        else
            if res ~= "restart" then
                error(res, 0)
            end
        end
    end
end

local
function animator_advance(animator, dt)
    for i = 1, #(animator.threads) do
        local thread = animator.threads[i]
        local accum = animator.accums[i]
        accum = accum - dt
        while accum < 0 do
            local success, res = coroutine.resume(thread, "continue")
            if success then
                if res == "finished" then
                    accum = 10000000000
                else
                    accum = accum + res
                end
            else
                error(res, 0)
            end
        end
        animator.accums[i] = accum
    end
    animator.tweens:Advance(dt)
end

local
function animator_restart(animator)
    for i, thread in ipairs(animator.threads) do
        local success, res = coroutine.resume(thread, "restart")
        if not success then
            error(res, 0)
        end
        animator.accums[i] = 0
    end
    animator.tweens:Clear()
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
        local thread = coroutine.create(thread_func)
        coroutine.resume(thread, f, tweens)
        threads[i] = thread
        accums[i] = 0
    end
    local animator = {
        tweens = tweens,
        threads = threads,
        accums = accums, -- amount of time to wait before resuming each thread
        Advance = animator_advance,
        Restart = animator_restart,
    }
    return animator
end

function lt.Wait(t)
    local cmd = coroutine.yield(t)
    if cmd == "continue" then
        return
    elseif cmd == "restart" then
        error("restart", 0)
    else
        error("unrecognised command: " .. cmd)
    end
end
