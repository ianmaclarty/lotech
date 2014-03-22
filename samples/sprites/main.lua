local main_layer = lt.Layer()

local images = {}
lt.AddSpriteFiles(images, "run", 15)

local frames = lt.LoadImages(images, "linear", "linear")
local sprite = lt.Sprite(lt.MatchFields(frames, "run_%d"), 40)

main_layer:Insert(sprite)

main_layer:Action(function(dt)
  lt.AdvanceGlobalSprites(dt)
end)

main_layer:KeyDown(function(event)
  if event.key == "esc" then
    lt.Quit()
  end
end)

lt.root.child = main_layer
