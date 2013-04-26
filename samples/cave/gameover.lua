local main_layer = lt.Layer()
lt.root:Insert(main_layer)

main_layer:KeyDown(function(event)
    import "main"
end)
