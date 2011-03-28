function clone(table)
    local copy = {}
    for k, v in pairs(table) do
        copy[k] = v
    end
    return copy
end

function lt.ClearFuncs()
    lt.Advance = nil
    lt.Render = nil
    lt.PointerDown = nil
    lt.PointerUp = nil
    lt.PointerMove = nil
    lt.KeyUp = nil
    lt.KeyDown = nil
end
