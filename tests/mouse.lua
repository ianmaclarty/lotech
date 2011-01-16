dofile("../src/ltlua/lt.lua")

lt.SetViewPort(-15, -10, 15, 10)

local mouse_x
local mouse_y
local down = false
local rects = {}

function add_rect(x, y)
    rect = {
        x1 = x - 1,
        y1 = y - 1,
        x2 = x + 1,
        y2 = y + 1
    }
    rects[rect] = true
end

function lt.MouseMove(x, y)
    mouse_x = x
    mouse_y = y
    if down then
        add_rect(x, y)
    end
end

function lt.MouseDown(button, x, y)
    if button == 1 then
        down = true
        add_rect(x, y)
    end
end

function lt.MouseUp(button, x, y)
    if button == 1 then
        down = false
    end
end

function lt.Render()
    lt.PushTint(0, 0, 1)
    for rect in next, rects do
        lt.DrawRect(rect.x1, rect.y1, rect.x2, rect.y2)
    end
    lt.PopTint()
    if down then
        lt.PushTint(1, 1, 0)
    else
        lt.PushTint(1, 0, 0)
    end
    if mouse_x and mouse_y then
        lt.DrawRect(mouse_x - 1, mouse_y - 1, mouse_x + 1, mouse_y + 1)
    end
    lt.PopTint()
end
