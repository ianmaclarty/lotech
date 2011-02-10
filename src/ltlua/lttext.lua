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

    -- Build an array of lines.
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

    -- Compute text bounding box.
    local bb = {w = 0}
    bb.h = #lines * vmove - (vmove - em.h)
    for _, line in ipairs(lines) do
        if line.w > bb.w then
            bb.w = line.w
        end
    end
    if halign == "left" then
        bb.l = 0
        bb.r = bb.w
    elseif halign == "right" then
        bb.l = -bb.w
        bb.r = 0
    else -- centered
        bb.l = -bb.w / 2
        bb.r = bb.w / 2
    end

    -- Insert lines into haligned node.
    local haligned = lt.Layer()
    for _, line in ipairs(lines) do
        if halign == "left" then
            haligned:Insert(line)
        elseif halign == "right" then
            haligned:Insert(lt.Translate(line, -line.w, 0))
        else -- centered
            haligned:Insert(lt.Translate(line, -line.w / 2, 0))
        end
    end

    -- Create valigned node.
    local valigned
    if valign == "top" then
        valigned = haligned
        bb.t = 0
        bb.b = -bb.h
    elseif valign == "bottom" then
        valigned = lt.Translate(haligned, 0, bb.h)
        bb.t = bb.h
        bb.b = 0
    else -- centered
        valigned = lt.Translate(haligned, 0, bb.h / 2)
        bb.t = bb.h / 2
        bb.b = -bb.h / 2
    end

    valigned.bb = bb
    return valigned
end
