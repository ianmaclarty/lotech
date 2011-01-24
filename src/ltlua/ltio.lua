local function wrt(ind, file, x)
    local t = type(x)
    local spaces = string.rep("  ", ind + 1)
    if t == "table" then
        file:write("{\n")
        for key, val in pairs(x) do
            if type(key) == "string" then
                file:write(spaces, key, " = ")
            else
                file:write(spaces, "[", key, "] = ")
            end
            wrt(ind + 1, file, val)
            file:write(",\n")
        end
        file:write(string.rep("  ", ind), "}")
    elseif t == "string" then
        file:write("[[", x, "]]")
    else
        file:write(tostring(x))
    end 
end

function lt.Write(file, x)
    wrt(0, file, x)
end

function lt.Save(filename, val)
    local file = io.open(filename, "w")
    file:write("return ")
    lt.Write(file, val)
    file:close()
end

function lt.Load(filename)
    local chunk = loadfile(filename)
    return chunk()
end
