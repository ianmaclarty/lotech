function lt.Text(str, font)
    local em = font.m or font.M or {w = 0.1, h = 0.1}
    local space = em.w * (font.space or 0.3)
    local hmove = em.w * (font.hmove or 0.05)
    local vmove = em.h * (font.vmove or 1.2)
    local kerntable = font.kern
    local x, y, dx, k, gap = 0, 0, 0, 0, 0
    local line = lt.Layer()
    local lines = {line}
    for i = 1, str:len() do
        local chr = str:sub(i, i)
        local kernpair = kerntable and str:sub(i, i + 1)
        if chr == "\n" then
            line.w = x - gap
            line = lt.Layer()
            table.insert(lines, line)
            y = y - vmove
            x = 0
        else
            local img = font[chr]
            if not img then
                dx = space
                gap = space
            else
                line:Insert(lt.Translate(img, x + img.w / 2, y))
                k = kerntable and kerntable[kernpair]
                gap = (k and k * em.w or hmove)
                dx = gap + img.w
            end
            x = x + dx
        end
    end
    line.w = x - gap
    local layer = lt.Layer()
    local boxwidth = 0
    for _, line in ipairs(lines) do
        layer:Insert(line)
        if line.w > boxwidth then
            boxwidth = line.w
        end
    end
    layer.w = boxwidth
    return layer
end
