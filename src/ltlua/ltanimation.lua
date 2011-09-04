-- Copyright 2011 Ian MacLarty
function lt.AnimatorSet()
    return {}
end

function lt.AddAnimator(animators, animator)
    local thread = coroutine.create(animator)
    local state = {
        thread = thread,
        -- The time remaining before the animator should be invoked again.
        t = 0,
    }
    table.insert(animators, state)
end

function lt.Animate(animators, time_step)
    local success
    local wait
    local died = {}
    for i, state in pairs(animators) do
        local is_dead = false
        local t = state.t - time_step
        while t <= 0 and not is_dead do
            success, wait = coroutine.resume(state.thread)
            if success == false then
                -- wait is the error message
                error(wait)
            end
            t = t + wait
            is_dead = coroutine.status(state.thread) == "dead"
        end
        if is_dead then
            animators[i] = nil
        else
            state.t = t
        end
    end
end

local yield = coroutine.yield
function lt.Wait(t)
    yield(t)
end
