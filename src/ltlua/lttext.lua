function lt.Text(str, font, halign, valign)
    halign = halign or "left"
    valign = valign or "center"
    local em = font.m or font.M or {w = 0.1, h = 0.1}
    local space = em.w * (font.space or 0.3)
    local hmove = em.w * (font.hmove or 0.05)
    local vmove = em.h * (font.vmove or 1.2)
    local kerntable = font.kern
    local x, y, dx, k, gap = 0, -em.h / 2, 0, 0, 0
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
    local haligned = lt.Layer()
    -- Compute text bounding box.
    local bb_w = 0
    local bb_h = #lines * vmove - (vmove - em.h)
    for _, line in ipairs(lines) do
        if line.w > bb_w then
            bb_w = line.w
        end
    end
    local bb_l, bb_r
    if halign == "left" then
        bb_l = 0
        bb_r = bb_w
    elseif halign == "right" then
        bb_l = -bb_w
        bb_r = 0
    else -- centered
        bb_l = -bb_w / 2
        bb_r = bb_w / 2
    end
    -- Insert lines into haligned layer.
    for _, line in ipairs(lines) do
        if halign == "left" then
            haligned:Insert(line)
        elseif halign == "right" then
            haligned:Insert(lt.Translate(line, -line.w, 0))
        elseif halign == "center" then
            haligned:Insert(lt.Translate(line, -line.w / 2, 0))
        end
    end
    local valigned
    if valign == "top" then
        valigned = haligned
        bb_t = 0
        bb_b = -bb_h
    elseif valign == "bottom" then
        valigned = lt.Translate(haligned, 0, bb_h)
        bb_t = bb_h
        bb_b = 0
    else -- centered
        valigned = lt.Translate(haligned, 0, bb_h / 2)
        bb_t = bb_h / 2
        bb_b = -bb_h / 2
    end
    valigned.w = bb_w
    valigned.h = bb_h
    valigned.l = bb_l
    valigned.r = bb_r
    valigned.t = bb_t
    valigned.b = bb_b
    return valigned
end
