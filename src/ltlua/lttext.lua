function lt.Text(str, font, halign, valign)
    halign = halign or "left"
    valign = valign or "center"

    local em = font.m or font.M or {width = 0.1, height = 0.1}
    local space = em.width * (font.space or 0.3)
    local hmove = em.width * (font.hmove or 0.05)
    local vmove = em.height * (font.vmove or 1.2)
    local kerntable = font.kern
    local x, y, dx, k, gap = 0, -em.height / 2, 0, 0, 0
    local line = lt.Layer()

    -- Build an array of lines.
    local lines = {line}
    for i = 1, str:len() do
        local chr = str:sub(i, i)
        local kernpair = kerntable and str:sub(i, i + 1)
        if chr == "\n" then
            line.width = x - gap
            line = lt.Layer()
            table.insert(lines, line)
            y = y - vmove
            x = 0
        else
            local img = font[chr] or font[string.upper(chr)] or font[string.lower(chr)]
            if not img then
                dx = space
                gap = space
            else
                line:Insert(lt.Translate(img, x + img.width / 2, y))
                k = kerntable and kerntable[kernpair]
                gap = (k and k * em.width or hmove)
                dx = gap + img.width
            end
            x = x + dx
        end
    end
    line.width = x - gap

    -- Compute text bounding box.
    bb_width = 0;
    bb_height = #lines * vmove - (vmove - em.height)
    for _, line in ipairs(lines) do
        if line.width > bb_width then
            bb_width = line.width
        end
    end
    if halign == "left" then
        bb_left = 0
        bb_right = bb_width
    elseif halign == "right" then
        bb_left = -bb_width
        bb_right = 0
    else -- centered
        bb_left = -bb_width / 2
        bb_right = bb_width / 2
    end

    -- Insert lines into haligned node.
    local haligned = lt.Layer()
    for _, line in ipairs(lines) do
        if halign == "left" then
            haligned:Insert(line)
        elseif halign == "right" then
            haligned:Insert(lt.Translate(line, -line.width, 0))
        else -- centered
            haligned:Insert(lt.Translate(line, -line.width / 2, 0))
        end
    end

    -- Create valigned node.
    local valigned
    if valign == "top" then
        valigned = haligned
        bb_top = 0
        bb_bottom = -bb_height
    elseif valign == "bottom" then
        valigned = lt.Translate(haligned, 0, bb_height)
        bb_top = bb_height
        bb_bottom = 0
    else -- centered
        valigned = lt.Translate(haligned, 0, bb_height / 2)
        bb_top = bb_height / 2
        bb_bottom = -bb_height / 2
    end

    valigned.width = bb_width
    valigned.height = bb_height
    valigned.left = bb_left
    valigned.bottom = bb_bottom
    valigned.right = bb_right
    valigned.top = bb_top
    return valigned
end
