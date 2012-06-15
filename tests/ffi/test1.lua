local t1 = test.TestStruct1()
print(t1.type)
print(t1.is["XXX"])
print(t1.int_field)
t1.int_field = 52
print(t1.int_field)
print(t1.float_field)
t1.float_field = -3.14
print(t1.float_field)
t1.enum_field = "val3"
print(t1.enum_field)
print("===")

local t2 = test.TestStruct2()
print(t2.type)
print(t2.is[t1.type])
print(t2.is["test.TestStruct1"])
print(t2.is["test.TestStruct2"])
print(t2.is["test.TestStruct3"])
print(t2.bool_field)
t2.bool_field = false
print(t2.bool_field)
print(t2.int_field)
t2.int_field = 54
print(t2.int_field)
print(t2.float_field)
t2.float_field = 19.1
print(t2.float_field)
print(t2.obj_field)
t2.obj_field = t1
print(t2.obj_field.int_field)
print("===")

local t3 = test.TestStruct2{
    bool_field = false,
    int_field = 111,
    obj_field = t2
}
print(t3.bool_field)
print(t3.int_field)
print(t3.float_field)
print(t3.obj_field.int_field)
print(t3.obj_field.obj_field.int_field)
t3:add_one()
print(t3.int_field)
print("===")

local t4 = test.TestStruct2(1, 2.5, "val1", false, t3)
print(t4.int_field)
print(t4.float_field)
print(t4.bool_field)
print(t4.enum_field)
print(t4.obj_field.int_field)
print(t4.obj_field.obj_field.int_field)
print(t4.obj_field.obj_field.obj_field.int_field)
t4:insert_obj(t3)
t4:insert_obj(t2)
t4:insert_obj(t3)
print(t4:count_obj())
t4:remove_obj(t2)
t4:remove_obj(t3)
t4:remove_obj(t3)
print(t4:count_obj())

print("===")

local t5 = test.TestStruct3(123)
print(t5.int_field)
print(t5.float_field)
t5.obj_field = t4
print(t5.obj_field.int_field)

print("===")

local t6 = test.TestStruct2(12, {float_field = 13})
print(t6.int_field)
print(t6.float_field)
