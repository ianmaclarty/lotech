function lt.DoInNewMatrix(f)
    lt.PushMatrix()
    f()
    lt.PopMatrix()
end
