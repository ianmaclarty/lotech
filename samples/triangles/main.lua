local mesh = lt.Mesh()
local xys = {
    200, 200,
    200, 300,
    300, 200,
}
local rgbs = {
    1, 0, 0,
    0, 1, 0,
    0, 0, 1
}

mesh:Action(function(dt)
    for i = 1, #xys, 2 do
        xys[i] = math.random(0, 960)
        xys[i + 1] = math.random(0, 640)
        rgbs[i] = math.random()
        rgbs[i + 1] = math.random()
        rgbs[i + 2] = math.random()
    end
    mesh:SetXYs(xys)
    mesh:SetRGBs(xys)
    return 0.5
end)

lt.root:Insert(mesh)
