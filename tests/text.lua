dofile("../src/ltlua/lt.lua")

local images = lt.LoadImages({
    {font = "font.png", glyphs = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz1234567890.,?!"},
})

local main = lt.Layer()

main:Insert(lt.Translate(lt.Scale(lt.Text("Hello World!", images.font), 3), -0.5, 0))

function lt.Render()
    main:Draw()
end
