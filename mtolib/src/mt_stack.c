#include "mt_stack.h"

void mt_stack_alloc_entries(mt_stack* stack)
{
  stack->entries =
    safe_malloc(MT_STACK_NUM_ENTRIES_BYTES(MT_STACK_INIT_SIZE));  
  stack->num_entries = 0;
  stack->max_entries = MT_STACK_INIT_SIZE; 
}

void mt_stack_free_entries(mt_stack* stack)
{
  free(stack->entries);
  
  stack->entries = NULL;
}

void mt_stack_resize(mt_stack* stack)
{
  stack->max_entries <<= 1;
  
  stack->entries =
    safe_realloc(stack->entries,
      MT_STACK_NUM_ENTRIES_BYTES(stack->max_entries));
}

void mt_stack_insert(mt_stack* stack, const mt_pixel* pixel)
{
  if (stack->max_entries == stack->num_entries)
  {
    mt_stack_resize(stack);
  }
  
  stack->entries[stack->num_entries] = *pixel;
  
  ++stack->num_entries;
}

const mt_pixel* mt_stack_remove(mt_stack* stack)
{
  return stack->entries + --stack->num_entries;
}
