#!/bin/bash

libdirectory="${0%/*}/mtolib/lib"

if [ ! -d $libdirectory ];
then mkdir $libdirectory
fi

cd "${0%/*}/mtolib/lib"

socount=`ls *.so 2>/dev/null | wc -l`
if [ $socount -gt 0 ]
then rm *.so
fi


cd "${0%/*}/../src"

gcc -shared -fPIC -include main.h -o ../lib/mt_objects.so mt_objects.c mt_heap.c mt_node_test_4.c
gcc -shared -fPIC -include main.h -o ../lib/maxtree.so maxtree.c mt_stack.c mt_heap.c

gcc -shared -fPIC -include main_double.h  -o ../lib/mt_objects_double.so mt_objects.c mt_heap.c mt_node_test_4.c
gcc -shared -fPIC -include main_double.h -o ../lib/maxtree_double.so maxtree.c mt_stack.c mt_heap.c
