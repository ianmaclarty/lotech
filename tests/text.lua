dofile("../src/ltlua/lt.lua")

lt.SetViewPort(-1, -1, 1, 1)

local images = lt.LoadImages({
    {font = "helvetica.png", glyphs = "ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890"},
})

local letters = lt.Layer()

letters:Insert(lt.Scale(images.helvetica.E, 1))
--letters:Insert(images.helvetica.B)
--letters:Insert(images.helvetica.B)
--letters:Insert(images.helvetica.C)

local main = lt.Scale(letters, 2)

function lt.Render()
    main:Draw()
end
