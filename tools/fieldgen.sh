#!/bin/sh
file_dir=`dirname $1`
file=`basename $1`
struct=$2
gawk -v struct=$struct -v file=$file '
BEGIN {
    in_struct = 0;
    struct_name = "";
    num_fields = 0;
}
/^struct / {
    if ($2 == struct) {
        in_struct = 1;
        struct_name = $2;
        num_fields = 0;
    }
}
/^ *LT(float|degrees|secs) / {
    if (in_struct && substr($2, length($2)) == ";") {
        field_name = substr($2, 1, length($2) - 1);
        field_names[num_fields] = field_name;
        field_records[num_fields] = sprintf("((size_t) ( (char *)&((%s *)(0))->%s - (char *)0 ))", struct, field_name);
        num_fields++;
    }
}
/^ *LTPoint / {
    if (in_struct && substr($2, length($2)) == ";") {
        field_name = substr($2, 1, length($2) - 1);
        field_names[num_fields] = field_name "_x";
        field_records[num_fields] = sprintf("((size_t) ( (char *)&((%s *)(0))->%s.x - (char *)0 ))", struct, field_name);
        num_fields++;
        field_names[num_fields] = field_name "_y";
        field_records[num_fields] = sprintf("((size_t) ( (char *)&((%s *)(0))->%s.y - (char *)0 ))", struct, field_name);
        num_fields++;
    }
}
/^ *LTColor / {
    if (in_struct && substr($2, length($2)) == ";") {
        field_name = substr($2, 1, length($2) - 1);
        field_names[num_fields] = field_name "_red";
        field_records[num_fields] = sprintf("((size_t) ( (char *)&((%s *)(0))->%s.r - (char *)0 ))", struct, field_name);
        num_fields++;
        field_names[num_fields] = field_name "_green";
        field_records[num_fields] = sprintf("((size_t) ( (char *)&((%s *)(0))->%s.g - (char *)0 ))", struct, field_name);
        num_fields++;
        field_names[num_fields] = field_name "_blue";
        field_records[num_fields] = sprintf("((size_t) ( (char *)&((%s *)(0))->%s.b - (char *)0 ))", struct, field_name);
        num_fields++;
        field_names[num_fields] = field_name "_alpha";
        field_records[num_fields] = sprintf("((size_t) ( (char *)&((%s *)(0))->%s.a - (char *)0 ))", struct, field_name);
        num_fields++;
    }
}
/^};$/ {
    if (in_struct) {
        print("%{");
        printf("#include \"ltcommon.h\"\n");
        printf("#include \"%s\"\n", file);
        print("%}");
        printf("struct LTFieldInfo { const char *name, size_t offset };\n");
        print("%%");
        for (i = 0; i < num_fields; i++) {
            printf("%s, %s\n",
                field_names[i], field_records[i]);
        }
        in_struct = 0;
    }
}' $file_dir/$file | gperf -tTE -LANSI-C -H ${struct}_field_hash -N ${struct}_field_info > $file_dir/${struct}_fields.h
