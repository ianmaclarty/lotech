-- Copyright 2011 Ian MacLarty
lt.frames_per_sec = 60
lt.secs_per_frame = 1 / lt.frames_per_sec
lt.radians_per_degree = math.pi / 180
lt.degrees_per_radian = 180 / math.pi

local sin_func__ = math.sin
local cos_func__ = math.cos
local rads_per_deg__ = lt.radians_per_degree
local degs_per_rad__ = lt.degrees_per_radian

-- Trigonometric functions that take degrees.
function sin(x)
    return sin_func__(x * rads_per_deg__)
end
function cos(x)
    return cos_func__(x * rads_per_deg__)
end
