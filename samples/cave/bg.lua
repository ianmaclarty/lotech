function make_cave_wall_bg(top_verts, bottom_verts, scale)
    local mesh = images.wall_edge:Mesh()
    mesh:SetXYs{}
    local edge = images.wall_edge:Mesh()
    local fill = images.wall_inner:Mesh()
    local h = 27
    local offset = 5
    local e = 0.01
    local noise = 3
    local divisions = 3
    local x = top_verts[1] * scale
    local y_top = top_verts[2] * scale
    local y_bottom = bottom_verts[2] * scale
    local xx, yy_top, yy_bottom
    for i = 1, #top_verts - 3, 2 do
        local x1 = top_verts[i] * scale
        local y1_top = top_verts[i + 1] * scale
        local y1_bottom = bottom_verts[i + 1] * scale
        local x2 = top_verts[i + 2] * scale
        local y2_top = top_verts[i + 3] * scale
        local y2_bottom = bottom_verts[i + 3] * scale
        local d = x2 - x1
        local g_top = (y2_top - y1_top) / d
        local g_bottom = (y2_bottom - y1_bottom) / d
        local x_inc = d / divisions
        local y_inc_top = g_top * x_inc
        local y_inc_bottom = g_bottom * x_inc

        for i = 1, divisions do
            yy_top = y_top + y_inc_top
            yy_bottom = y_bottom + y_inc_bottom
            xx = x + x_inc

            edge:SetXYs{
                x - e, y_top - offset,
                xx + e, yy_top - offset,
                xx + e, yy_top + h,
                x - e, y_top + h,
            }
            mesh:Merge(edge)

            local y3 = y_top
            local y4 = yy_top
            for j = 1, 7 do
                y3 = y3 + h
                y4 = y4 + h
                fill:SetXYs{
                    x - e, y3 - offset,
                    xx + e, y4 - offset,
                    xx + e, y4 + h,
                    x - e, y3 + h,
                }
                mesh:Merge(fill)
            end

            edge:SetXYs{
                xx + e, yy_bottom + offset,
                x - e, y_bottom + offset,
                x - e, y_bottom - h,
                xx + e, yy_bottom - h,
            }
            mesh:Merge(edge)

            y3 = y_bottom
            y4 = yy_bottom
            for j = 1, 7 do
                y3 = y3 - h
                y4 = y4 - h
                fill:SetXYs{
                    x - e, y3 + offset,
                    xx + e, y4 + offset,
                    xx + e, y4 - h,
                    x - e, y3 - h,
                }
                mesh:Merge(fill)
            end

            x = xx
            y_top = yy_top
            y_bottom = yy_bottom
        end
    end

    return mesh
end
