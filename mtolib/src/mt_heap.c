#include "mt_heap.h"

// Heap

void mt_heap_alloc_entries(mt_heap* heap)
{
  heap->entries =
    safe_malloc(MT_HEAP_NUM_ENTRIES_BYTES(MT_HEAP_INIT_SIZE));
  heap->num_entries = 0;
  heap->max_entries = MT_HEAP_INIT_SIZE; 
}

void mt_heap_free_entries(mt_heap* heap)
{
  free(heap->entries);
  
  heap->entries = NULL;
}

void mt_heap_resize(mt_heap* heap)
{
  heap->max_entries <<= 1;
  
  heap->entries = safe_realloc(heap->entries,
    MT_HEAP_NUM_ENTRIES_BYTES(heap->max_entries));
}

void mt_heap_insert(mt_heap* heap, const mt_pixel* pixel)
{
  if (heap->max_entries == heap->num_entries)
  {
    mt_heap_resize(heap);
  }
  
  INT_TYPE index = heap->num_entries + 1;
  mt_pixel* entry = heap->entries + index;
  PIXEL_TYPE pix_value = pixel->value;
  
  // Up-heap.
  
  while (index != 1)  
  {
    INT_TYPE up_index = MT_HEAP_UP(index);
    mt_pixel* up_entry = heap->entries + up_index;
    
    if (up_entry->value >= pix_value)
    {
      break;
    }
    
    *entry = *up_entry;
    
    index = up_index;
    entry = up_entry;
  }
  
  *entry = *pixel;
  
  ++heap->num_entries; 
}

const mt_pixel* mt_heap_remove(mt_heap* heap)
{
  INT_TYPE index = 1;                         
  mt_pixel* entry = heap->entries + index;    
  mt_pixel root_entry = *entry;               // Save root entry.
  
                                              // Entry to down-heap.
  mt_pixel* last_entry = heap->entries + heap->num_entries;
  
  // Down-heap.
  
  while (TRUE)
  {
    index = MT_HEAP_DOWN(index, 0);
    
    if (index > heap->num_entries)
    {
      break;
    }
    
    mt_pixel* down = heap->entries + index; // Left branch.
    
    if (index < heap->num_entries)
    {
      mt_pixel* down_right = down + 1;      // Right branch.
    
      if (down_right->value > down->value)
      {
        ++down;
        ++index;
      }
    }      
   
    if (down->value <= last_entry->value)
    {
      break;
    }
    
    *entry = *down;
    entry = down;
  }
  
  *entry = *last_entry;
  *last_entry = root_entry;
  
  --heap->num_entries;
    
  return last_entry;
}
