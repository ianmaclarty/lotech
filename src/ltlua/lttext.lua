function lt.Text(str, font)
    local layer = lt.Layer()
    local x = 0
    local y = 0
    local em = font.m or font.M or {w = 0.1, h = 0.1}
    local space = em.w * (font.space or 0.3)
    local hmove = em.w * (font.hmove or 0.05)
    local vmove = em.h * (font.vmove or 1.2)
    local kern = font.kern
    for i = 1, str:len() do
        local chr = str:sub(i, i)
        local kernpair
        if kern then
            kernpair = str:sub(i, i + 1)
        end
        if chr == "\n" then
            y = y - vmove
            x = 0
        else
            local img = font[chr]
            if not img then
                x = x + space
            else
                local w2 = img.w / 2
                x = x + w2
                layer:Insert(lt.Translate(img, x, y))
                local k = hmove
                if kern then
                    k = kern[kernpair]
                    if k then
                        k = k * em.w
                    else
                        k = hmove
                    end
                end
                x = x + w2 + k
            end
        end
    end
    return layer
end
