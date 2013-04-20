-- Copyright 2011 Ian MacLarty
local sprites_mt = {__mode = "v"}

-------------------------------------------------------------

function lt.SpriteSet()
    local sprites = {}
    setmetatable(sprites, sprites_mt)
    return sprites
end

local global_sprites = lt.SpriteSet()

local
function sprite_reset(sprite)
    sprite.child = sprite.frames[1]
    sprite.t_accum = 0
    sprite.curr_frame = 1
end

-- frames is an array of images.
function lt.Sprite(frames, fps, spriteset)
    if not spriteset then
        spriteset = global_sprites
    end
    local n = #spriteset
    local sprite = lt.Wrap(frames[1])
    sprite.frames = frames
    sprite.spf = 1 / fps -- secs per frame
    sprite.t_accum = 0
    sprite.curr_frame = 1
    sprite.paused = false
    sprite.loop = true
    sprite.Reset = sprite_reset
    spriteset[n + 1] = sprite
    return sprite
end

function lt.AdvanceSprites(spriteset, step)
    local t_accum
    local spf -- secs per frame
    local num_frames
    local frames
    local curr_frame
    local prev_frame
    for _, sprite in pairs(spriteset) do
        if not sprite.paused then
            spf = sprite.spf
            t_accum = sprite.t_accum + step
            frames = sprite.frames
            num_frames = #frames
            curr_frame = sprite.curr_frame
            prev_frame = curr_frame
            while t_accum >= spf do
                t_accum = t_accum - spf
                curr_frame = curr_frame + 1
                if curr_frame > num_frames then
                    if sprite.loop then
                        curr_frame = type(sprite.loop) == "number" and sprite.loop or 1
                    else
                        curr_frame = num_frames
                    end
                end
            end
            sprite.t_accum = t_accum
            if prev_frame ~= curr_frame then
                sprite.curr_frame = curr_frame
                sprite.child = frames[curr_frame]
            end
        end
    end
end

function lt.AdvanceGlobalSprites(dt)
    lt.AdvanceSprites(global_sprites, dt or lt.secs_per_frame)
end

function lt.ClearGlobalSprites()
    global_sprites = lt.SpriteSet()
end
