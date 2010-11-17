function lt.Scene()
    return {}
end

-- Elements with higher z values are drawn on top of elements
-- with lower z values.
function lt.AddToScene(scene, draw, z)
    scene[draw] = z
end

function lt.RemoveFromScene(scene, draw)
    scene[draw] = nil
end

function lt.ReplaceInScene(scene, old, new)
    local z = scene[old]
    scene[old] = nil
    scene[new] = z
end

function lt.DrawScene(scene)
    -- Sort elements by z
    local i = 1
    local arr = {}
    for draw, z in pairs(scene) do
        arr[i] = {z = z, draw = draw}
        i = i + 1
    end
    table.sort(arr, function(x, y) return x.z < y.z end)

    -- Draw the elements.
    for j, entry in ipairs(arr) do
        entry.draw()
    end
end
