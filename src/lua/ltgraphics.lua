-- Copyright (C) 2010-2013 Ian MacLarty. See Copyright Notice in ../lt.h
-- Copyright (C) 2010-2013 Ian MacLarty. See Copyright Notice in ../lt.h
function lt.DoInNewMatrix(f)
    lt.PushMatrix()
    f()
    lt.PopMatrix()
end
