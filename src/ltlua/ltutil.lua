-- Logging to stderr.
function log(msg)
    io.stderr:write(msg .. "\n")
end

function clone(table)
    local copy = {}
    for k, v in pairs(table) do
        copy[k] = v
    end
    return copy
end
