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
    local hit_filter = node:HitFilter(left, bottom, right, top)
    local hit_filter_wrap = hit_filter:Wrap()
    local button = hit_filter_wrap:Wrap()
    hit_filter:OnPointerDown(function(down_input, down_x, down_y)
        if hit_filter.input then
            return false
        end
        hit_filter.input = down_input
        hit_filter_wrap:OnPointerUp(function(up_input, up_x, up_y)
            if hit_filter.input == up_input then
                if onUp then
                    onUp()
                end
                hit_filter_wrap = hit_filter:Wrap()
                button:Replace(hit_filter_wrap)
                hit_filter.input = nil
                return true
            end
            return false
        end)
        if onDown then
            onDown()
        end
        return true
    end)
    return button
end

function lt.HitBarrier(child)
    return lt.HitFilter(child, 1, 1, -1, -1)
end
