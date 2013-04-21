#!/bin/sh
years="2010-2013"

for f in `ls src/*.h src/*.cpp src/*.mm | grep -v "lt\.h"`; do
    echo "/* Copyright (C) $years Ian MacLarty. See Copyright Notice in lt.h. */" > cp.tmp
    cat $f | grep -v "Copyright .* Ian MacLarty\. See Copyright Notice in" >> cp.tmp
    mv cp.tmp $f
done
for f in `ls src/lua/*.lua src/lua/*.lua.new`; do
    echo "-- Copyright (C) $years Ian MacLarty. See Copyright Notice in ../lt.h" > cp.tmp
    cat $f | grep -v "Copyright .* Ian MacLarty\. See Copyright Notice in" >> cp.tmp
    mv cp.tmp $f
done
