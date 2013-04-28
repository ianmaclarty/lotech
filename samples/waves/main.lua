local mesh = lt.Mesh()
local xyzs = {}
local indices = {}
local n, m = 80, 50
local w, d = 4800, 100
local shift = w / 2 - lt.config.world_width / 2
local i = 1
for x = 1, n do
    for z = 1, m do
        table.append(xyzs, {((x - 1) / (n - 1)) * w - shift, 0, -((z - 1) / (m - 1)) * d})
        if x < n and z < m then
            -- counter-clockwise winding for front faces
            table.append(indices, {
                i, i + m, i + 1,
                i + m, i + m + 1, i + 1,
            })
        end
        i = i + 1
    end
end

mesh:SetXYZs(xyzs)
mesh:SetIndices(indices)
mesh:ComputeNormals()

local t = 0
local amplitude = 50
mesh:Action(function(dt)
    t = t + dt
    for x = 1, n do
        for z = 1, m do
            local i = (x - 1) * m + z
            i = (i - 1) * 3 + 2
            xyzs[i] = 
                (sin(((x - 1) / (n - 1)) * w * 0.7 + t * 100)
                + sin(((z - 1) / (m - 1)) * d * 10 + t * 100)) * amplitude
                - 2 * amplitude
        end
    end
    mesh:SetXYZs(xyzs)
    mesh:ComputeNormals()
end)

local light = mesh
    :CullFace("back")
    :DepthTest()
    :Light{x = 0, y = 0, z = 0, fixed=false, c = 1}

light:Action(function(dt, l)
    l:Tween{
        diffuse_red = math.random(),
        diffuse_green = math.random(),
        diffuse_blue = math.random(),
        ambient_red = math.random(),
        ambient_green = math.random(),
        ambient_blue = math.random(),
        specular_red = math.random(),
        specular_green = math.random(),
        specular_blue = math.random(),
        time = 1
    }
    return 1
end)

lt.root.child = light:Lighting():Perspective(1, 25, 200)
