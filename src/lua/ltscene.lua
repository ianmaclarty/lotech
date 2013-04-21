-- Copyright (C) 2010-2013 Ian MacLarty. See Copyright Notice in ../lt.h
lt.root = lt.Layer()
lt.root:Activate()

function lt.Render()
    lt.root:Draw()
end

function lt.Advance(dt)
    lt.ExecuteActions(dt)
end

function lt.HandleEvent(event)
    lt.root:PropagateEvent(event)
end
