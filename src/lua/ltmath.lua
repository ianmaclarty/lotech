-- Copyright (C) 2010-2013 Ian MacLarty. See Copyright Notice in ../lt.h
lt.frames_per_sec = 60
lt.secs_per_frame = 1 / lt.frames_per_sec
lt.radians_per_degree = math.pi / 180
lt.degrees_per_radian = 180 / math.pi

local sin_func = math.sin
local cos_func = math.cos
local tan_func = math.tan
local rads_per_deg = lt.radians_per_degree
local degs_per_rad = lt.degrees_per_radian

-- Trigonometric functions that take degrees.
function sin(x)
    return sin_func(x * rads_per_deg)
end
function tan(x)
    return tan_func(x * rads_per_deg)
end
function cos(x)
    return cos_func(x * rads_per_deg)
end
