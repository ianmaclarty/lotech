-- Copyright (C) 2010-2013 Ian MacLarty. See Copyright Notice in ../lt.h
local global_timers = {}

local insert = table.insert

function lt.Timer(t, func, timers)
    if timers == nil then
        timers = global_timers
    end
    insert(timers, {action = func, t = t})
end

function lt.AdvanceTimers(dt, timers)
    for i, tmr in pairs(timers) do
        local t = tmr.t
        t = t - dt
        if t <= 0 then
            tmr.action()
            timers[i] = nil
        else
            tmr.t = t
        end
    end
end

local ad = lt.AdvanceTimers

function lt.AdvanceGlobalTimers(dt)
    ad(dt, global_timers)
end

function lt.ClearGlobalTimers()
    global_timers = {}
end
