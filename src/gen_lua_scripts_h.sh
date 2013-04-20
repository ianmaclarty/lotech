#!/bin/sh

h=lua_scripts.h
cat <<EOF > $h
EOF

for f in `ls lua/*.lua`; do
    name=`basename $f .lua`
    echo "static const char *lt_script_$name = " >> $h
    cat $f | sed 's/\\/\\\\/g' | sed 's/"/\\"/g' | sed 's/^/"/' | sed 's/$/\\n"/' >> $h
    echo ";" >> $h
    echo >> $h
done
