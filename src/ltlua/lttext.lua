function lt.Text(str, font)
    local layer = lt.Layer()
    local x = 0
    local y = 0
    local em = font.m or font.M or {w = 0.1, h = 0.1}
    local h = em.h
    local space = em.w / 3
    local hgap = em.w / 32
    local ymove = h * 1.2
    for i = 1, str:len() do
        local chr = str:sub(i, i)
        if chr == "\n" then
            y = y - ymove
            x = 0
        else
            local img = font[chr]
            if not img then
                x = x + space
            else
                local w2 = img.w / 2
                x = x + w2
                layer:Insert(lt.Translate(img, x, y))
                x = x + w2 + hgap
            end
        end
    end
    return layer
end
