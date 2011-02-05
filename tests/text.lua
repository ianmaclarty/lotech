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

main:Insert(lt.Translate(lt.Scale(lt.Text("Hello World!\nVooV.", images.font), 3), -0.5, 0))

function lt.Render()
    main:Draw()
end
