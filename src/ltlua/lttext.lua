-- Copyright 2011 Ian MacLarty
function lt.Text(str, font, halign, valign)
    halign = halign or "left"
    valign = valign or "center"

    local em = font.m or font.M or font["0"] or {width = 0.1, height = 0.1}
    local space = em.width * (font.space or 0.3)
    local hmove = em.width * (font.hmove or 0.05)
    local vmove = em.height * (font.vmove or 1.2)
    local kerntable = font.kern
    local fixed_w = font.fixed and em.width
    local x, y, dx, k, gap = 0, -em.height / 2, 0, 0, 0
    local line = lt.Layer()
    local scale = 1

    -- Build an array of lines.
    local lines = {line}
    local i = 1
    local len = str:len()
    while i <= len do
        local chr = str:sub(i, i)
        local kernpair = kerntable and str:sub(i, i + 1)
        if chr == "\n" then
            line.width = x - gap
            line = lt.Layer()
            table.insert(lines, line)
            y = y - vmove
            x = 0
        elseif chr == "\\" then
            i = i + 1
            chr = str:sub(i, i)
            if chr == "+" then
                scale = scale + 0.1
            elseif chr == "-" then
                scale = scale - 0.1
            end
        else
            local img = font[chr] or font[string.upper(chr)] or font[string.lower(chr)]
            if not img then
                dx = space
                gap = space
            else
                local w = fixed_w or img.width
                local node = img
                if scale ~= 1 then
                    node = node:Scale(scale)
                end
                line:Insert(node:Translate(x + w/2, y))
                k = kerntable and kerntable[kernpair]
                gap = (k and k * em.width or hmove)
                dx = gap + w
            end
            x = x + scale * dx
        end
        i = i + 1
    end
    line.width = x - gap

    -- Compute text bounding box.
    local bb_width = 0;
    local bb_height = #lines * vmove - (vmove - em.height)
    local bb_top, bb_left, bb_bottom, bb_right
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
