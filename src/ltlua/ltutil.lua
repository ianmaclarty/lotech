function clone(table)
    local copy = {}
    for k, v in pairs(table) do
        copy[k] = v
    end
    return copy
end
