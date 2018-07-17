#include "maxtree.h"

#ifndef MT_STACK_H
#define MT_STACK_H

#define MT_STACK_NUM_ENTRIES_BYTES(X) ((X) * sizeof(mt_pixel))

#define MT_STACK_INIT_SIZE 1024

#define MT_STACK_TOP(MT_STACK_PTR) ((MT_STACK_PTR)->entries + (MT_STACK_PTR)->num_entries - 1)
#define MT_STACK_SIZE(MT_STACK_PTR) ((MT_STACK_PTR)->num_entries)
#define MT_STACK_NOT_EMPTY(MT_STACK_PTR) ((MT_STACK_PTR)->num_entries > 0)

typedef struct 
{
  mt_pixel *entries;
  INT_TYPE num_entries;
  INT_TYPE max_entries;
} mt_stack;

void mt_stack_alloc_entries(mt_stack* stack);
void mt_stack_free_entries(mt_stack* stack);
void mt_stack_insert(mt_stack* stack, const mt_pixel* pixel);
const mt_pixel* mt_stack_remove(mt_stack* stack);


#endif
