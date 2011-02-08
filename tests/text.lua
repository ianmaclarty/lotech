import "lt"

local images = lt.LoadImages({
    {font = "font", glyphs = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz1234567890.,?!"},
})

images.font.kern = {
    Wo = 0.01,
    Vo = 0.005,
    oV = 0.005,
    ["V."] = -0.15,
}
local main = lt.Layer()

local text = lt.Text("Hello World!\nVooV.", images.font)
local text_and_rect = lt.Layer()
text_and_rect:Insert(text, 2)
text_and_rect:Insert(lt.Tint(lt.Rect(0, 0.1, text.w, -0.1), 1, 0, 0, 0.5), 1)
main:Insert(lt.Translate(lt.Scale(text_and_rect, 3), -0.5, 0))

function lt.Render()
    main:Draw()
end
