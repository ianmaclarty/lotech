dofile("../src/ltlua/lt.lua")

images = lt.LoadImages({
    {"helvetica.png", "ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890"},
})

function lt.Render()
    images.helvetica.X:Draw()
end
