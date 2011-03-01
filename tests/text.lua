import "lt"

local images = lt.LoadImages({
    {font = "font", glyphs = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz1234567890.,?!"},
})

images.font.kern = {
    Wo = 0.01,
    Vo = 0.005,
    oV = 0.005,
    Vj = -0.04,
    ["V."] = -0.15,
}
local main = lt.Layer()

local text = lt.Text("Hello World!\nVjoV.", images.font, "center", "center")
local text_and_rect = lt.Layer()
text_and_rect:Insert(text, 2)
text_and_rect:Insert(lt.Tint(lt.Rect(text.bb.l, text.bb.b, text.bb.r, text.bb.t), 1, 0, 0, 0.5), 1)
main:Insert(lt.Scale(text_and_rect, 8))
main:Insert(lt.Line(0, -1, 0, 1), -1)
main:Insert(lt.Line(-1, 0, 1, 0), -1)

function lt.Render()
    main:Draw()
end
