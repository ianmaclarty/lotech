local
function test(str)
    log("----")
    local v, err = lt.FromJSON(str)
    if v == nil and str ~= "null" then
        if not err then
            log("Error parsing " .. str .. " but no error message returned")
        else 
            log("Error parsing " .. str .. ": " .. err)
        end
    else
        local s = lt.ToJSON(v)
        log(str)
        log(s)
    end
end
test("123.4")
test("{}")
test("[ ]")
test("[ true ]")
test("[1, 2, 3]")
test("[[\"a\",\"b\"  ,\"c\" , \"d\"], []]")
test("[[\"a\",\"b\"  ,\"c\" , \"d\"], [], \"ee\\ne\", \"\\\"\", \"xxxx\\\"xxxx\"]")
test("\"\"")
test("\"string\"")
test("true")
test("false")
test("null")
test("{\"name\":\"Ian\"}")
test("{\"stuff\": {\"field1\": [11.0,22.1,33.4], \"field2\": true}, \"more_stuff\": [true, true, {\"blah\": -99.99}]}")
