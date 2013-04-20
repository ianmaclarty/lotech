-- Copyright 2011 Ian MacLarty
local current_button_input
-- node:Button(bbox, onDown, onUp) or
-- node:Button(onDown, onUp) (node itself is the bbox)
-- node:Button(onUp) (node itself is the bbox)
function lt.Button(node, ...)
    local bbox, onDown, onUp
    local nargs = select("#", ...)
    if nargs == 3 then
        bbox, onDown, onUp = ...
    elseif nargs == 2 then
        bbox = node
        onDown, onUp = ...
    elseif nargs == 1 then
        bbox = node
        onUp = ...
    else
        error("wrong number of argument")
    end
    local left = bbox.left
    local bottom = bbox.bottom
    local right = bbox.right
    local top = bbox.top
    local button = node:DownFilter(left, bottom, right, top)
    button:OnPointerDown(function(down_input, down_x, down_y)
        if current_button_input then -- already a button being pressed
            return false
        end
        current_button_input = down_input
        button.input_id = down_input
        if onDown then
            onDown()
        end
        return true
    end)
    button:OnPointerUp(function(up_input, up_x, up_y)
        if current_button_input == up_input and button.input_id == up_input then
            if onUp then
                onUp()
            end
            current_button_input = nil
            button.input_id = nil
            return true
        end
        return false
    end)
    return button
end

function lt.HitBarrier(child)
    return lt.HitFilter(child, 1, 1, -1, -1)
end

function lt.CancelButton(input_id)
    if current_button_input == input_id then
        current_button_input = nil
    end
end
