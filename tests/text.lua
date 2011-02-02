dofile("../src/ltlua/lt.lua")

local images = lt.LoadImages({
    "helvetica.png",
    --{font = "helvetica.png", glyphs = "ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890"},
})

lt.Write(io.stderr, images)

local letters = lt.Layer()

letters:Insert(images.helvetica)
--letters:Insert(images.helvetica.B)
--letters:Insert(images.helvetica.B)
--letters:Insert(images.helvetica.C)

local main = lt.Scale(letters, 2)

function lt.Render()
    main:Draw()
end
