-- node:Button(bbox, onDown, onHit, onMiss) or
-- node:Button(onDown, onHit, onMiss) (node itself is the bbox)
function lt.Button(node, arg1, arg2, arg3, arg4)
    local bbox, onDown, onHit, onMiss
    local arg1type = type(arg1)
    if arg1type == "table" then
        bbox = arg1
        onDown = arg2
        onHit = arg3
        onMiss = arg4
    elseif arg1type == "function" then
        bbox = node
        onDown = arg1
        onHit = arg2
        onMiss = arg3
    else
        error("argument 1 should be a table or a function")
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
            return
        end
        hit_filter.input = down_input
        hit_filter_wrap:OnPointerUp(function(up_input, up_x, up_y)
            if hit_filter.input == up_input then
                if up_x >= left and up_x <= right and up_y >= bottom and up_y <= top then
                    onHit()
                else
                    onMiss()
                end
                hit_filter_wrap = hit_filter:Wrap()
                button:Replace(hit_filter_wrap)
                hit_filter.input = nil
            end
        end)
        onDown()
    end)
    return button
end