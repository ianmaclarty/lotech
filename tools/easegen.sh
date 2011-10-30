#!/bin/sh
file_dir=`dirname $1`
file=`basename $1`
gawk '
BEGIN {
    print("%{");
    print("#include \"lttween.h\"");
    print("%}");
    print("struct LTEaseFuncInfo { const char *name, LTEaseFunc func };");
    print("%%");
}
/^LTfloat ltEase_/ {
    f = substr($2, 8)
    printf("%s, ltEase_%s\n", f, f);
    next;
}
// {
    next;
}
' $file_dir/$file | gperf -tTE -LANSI-C -H LTEaseFunc_hash -N LTEaseFunc_lookup > $file_dir/lteasefunchash.h
