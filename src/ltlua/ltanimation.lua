function lt.AnimatorSet()
    return {}
end

function lt.AddAnimator(animators, animator)
    local thread = coroutine.create(animator)
    -- The value is the time remaining before the animator should be invoked again.
    animators[thread] = 0 
end

function lt.Animate(animators, time_step)
    local success
    local wait
    for animator, t in pairs(animators) do
        local is_dead = false
        t = t - time_step
        while t <= 0 and not is_dead do
            success, wait = coroutine.resume(animator)
            if success == false then
                -- wait is the error message
                error(retval)
            end
            t = t + wait
            is_dead = coroutine.status(animator) == "dead"
        end
        if is_dead then
            animators[animator] = nil
        else
            animators[animator] = t
        end
    end
end
