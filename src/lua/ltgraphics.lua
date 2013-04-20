-- Copyright 2011 Ian MacLarty
function lt.DoInNewMatrix(f)
    lt.PushMatrix()
    f()
    lt.PopMatrix()
end
