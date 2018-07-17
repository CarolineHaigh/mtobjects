#include "maxtree.h"

#ifndef MT_HEAP_H
#define MT_HEAP_H

#define MT_HEAP_DOWN(INDEX, N) (((INDEX) << 1) + (N))
#define MT_HEAP_UP(INDEX) ((INDEX) >> 1)

#define MT_HEAP_NUM_ENTRIES_BYTES(X) (((X) + 1) * sizeof(mt_pixel))

#define MT_HEAP_INIT_SIZE 1024

#define MT_HEAP_TOP(MT_HEAP_PTR) ((MT_HEAP_PTR)->entries + 1)
#define MT_HEAP_SIZE(MT_HEAP_PTR) ((MT_HEAP_PTR)->num_entries)
#define MT_HEAP_NOT_EMPTY(MT_HEAP_PTR) ((MT_HEAP_PTR)->num_entries > 0)
#define MT_HEAP_EMPTY(MT_HEAP_PTR) ((MT_HEAP_PTR)->num_entries == 0)

typedef struct
{
  mt_pixel* entries;
  INT_TYPE num_entries;
  INT_TYPE max_entries;
} mt_heap;

void mt_heap_alloc_entries(mt_heap* heap);
void mt_heap_free_entries(mt_heap* heap);

void mt_heap_insert(mt_heap* heap, const mt_pixel* pixel);
const mt_pixel* mt_heap_remove(mt_heap* heap);

#endif
