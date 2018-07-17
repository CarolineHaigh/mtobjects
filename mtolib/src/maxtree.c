#include "maxtree.h"

#include <assert.h>

#define MT_INDEX_OF(PIXEL) ((PIXEL).location.y * mt->img.width + \
  (PIXEL).location.x)

const int mt_conn_12[MT_CONN_12_HEIGHT * MT_CONN_12_WIDTH] =
{
  0, 0, 1, 0, 0,
  0, 1, 1, 1, 0,
  1, 1, 0, 1, 1,
  0, 1, 1, 1, 0,
  0, 0, 1, 0, 0
};

const int mt_conn_8[MT_CONN_8_HEIGHT * MT_CONN_8_WIDTH] =
{
  1, 1, 1,
  1, 0, 1,
  1, 1, 1,
};

const int mt_conn_4[MT_CONN_4_HEIGHT * MT_CONN_4_WIDTH] =
{
  0, 1, 0,
  1, 0, 1,
  0, 1, 0,
};

mt_pixel mt_starting_pixel(mt_data* mt)
{
  // Find the minimum pixel value in the image
  SHORT_TYPE y;
  SHORT_TYPE x;

  mt_pixel pixel;
  pixel.location.x = 0;
  pixel.location.y = 0;

  pixel.value = mt->img.data[0];

  // iterate over image pixels
  for (y = 0; y != mt->img.height; ++y)
  {
    for (x = 0; x != mt->img.width; ++x)
    {
	  // Convert from x,y coordinates to an array index
      INT_TYPE index = y * mt->img.width + x;

      // If the pixel is less than the current minimum, update the minimum
      if (mt->img.data[index] < pixel.value)
      {
        pixel.value = mt->img.data[index];
        pixel.location.x = x;
        pixel.location.y = y;

      }
    }
  }

  return pixel;
}

static void mt_init_nodes(mt_data* mt)
{
  // Initialise the nodes of the maxtree
  INT_TYPE i;

  // Set all parents as unassigned, and all areas as 1
  for (i = 0; i != mt->img.size; ++i)
  {
    mt->nodes[i].parent = MT_UNASSIGNED;
    mt->nodes[i].area = 1;
  }
}

static int mt_queue_neighbour(mt_data* mt, PIXEL_TYPE val,
  SHORT_TYPE x, SHORT_TYPE y)
{
  // Add a pixel to the queue for processing

  //Create a pixel and set its location
  mt_pixel neighbour;
  neighbour.location.x = x;
  neighbour.location.y = y;

  // Convert from x,y coordinates to an array index
  INT_TYPE neighbour_index = MT_INDEX_OF(neighbour);
  // Get a pointer to the neighbour
  mt_node *neighbour_node = mt->nodes + neighbour_index;

  // If the neighbour has not already been processed, add it to the queue
  if (neighbour_node->parent == MT_UNASSIGNED)
  {
    neighbour.value = mt->img.data[neighbour_index];
    neighbour_node->parent = MT_IN_QUEUE;
    mt_heap_insert(&mt->heap, &neighbour);

    // If the neighbour has a higher value than the current node, return 1
    if (neighbour.value > val)
    {
      return 1;
    }
  }

  return 0;
}

static void mt_queue_neighbours(mt_data* mt,
  mt_pixel* pixel)
{
  // Seems to be limiting conn values within image coordinates

  // Radius is half size of connectivity
  INT_TYPE radius_y = mt->connectivity.height / 2;
  INT_TYPE radius_x = mt->connectivity.width / 2;

  // If pixel's x is less than radius, conn = the difference
  INT_TYPE conn_x_min = 0;
  if (pixel->location.x < radius_x)
    conn_x_min = radius_x - pixel->location.x;

  // Ditto for y
  INT_TYPE conn_y_min = 0;
  if (pixel->location.y < radius_y)
    conn_y_min = radius_y - pixel->location.y;

  // If pixel's x + radius > image width, conn = radius + width - location - 1
  INT_TYPE conn_x_max = 2 * radius_x;
  if (pixel->location.x + radius_x >= mt->img.width)
    conn_x_max = radius_x + mt->img.width - pixel->location.x - 1;

  INT_TYPE conn_y_max = 2 * radius_y;
  if (pixel->location.y + radius_y >= mt->img.height)
    conn_y_max = radius_y + mt->img.height - pixel->location.y - 1;

  INT_TYPE conn_y;
  // Conn coordinates refer to position with connectivity grid
  for (conn_y = conn_y_min; conn_y <= conn_y_max; ++conn_y)
  {
    INT_TYPE conn_x;
    for (conn_x = conn_x_min; conn_x <= conn_x_max; ++conn_x)
    {
	  // Skip iteration if 0 in connectivity grid
      if (mt->connectivity.
        neighbors[conn_y * mt->connectivity.width + conn_x] == 0)
      {
        continue;
      }

      // Try to queue neighbour at x = x-rad+conn
      // If successfully queued and value higher than current,
      // break out of function
      if (mt_queue_neighbour(mt, pixel->value,
        pixel->location.x - radius_x + conn_x,
        pixel->location.y - radius_y + conn_y))
      {
        return;
      }
    }
  }
}

static void mt_merge_nodes(mt_data* mt,
  INT_TYPE merge_to_idx,
  INT_TYPE merge_from_idx)
{
  // Merge two nodes

  mt_node *merge_to = mt->nodes + merge_to_idx;
  mt_node_attributes *merge_to_attr = mt->nodes_attributes +
    merge_to_idx;

  mt_node *merge_from = mt->nodes + merge_from_idx;
  mt_node_attributes *merge_from_attr = mt->nodes_attributes +
    merge_from_idx;

  merge_to->area += merge_from->area;

  FLOAT_TYPE delta = mt->img.data[merge_from_idx] -
    mt->img.data[merge_to_idx];

  merge_from_attr->power += delta *
    (2 * merge_from_attr->volume + delta * merge_from->area);
  merge_to_attr->power += merge_from_attr->power;

  merge_from_attr->volume += delta * merge_from->area;
  merge_to_attr->volume += merge_from_attr->volume;
}

static void mt_descend(mt_data* mt, mt_pixel *next_pixel)
{
  mt_pixel old_top = *mt_stack_remove(&mt->stack);
  INT_TYPE old_top_index = MT_INDEX_OF(old_top);


  mt_pixel* stack_top = MT_STACK_TOP(&mt->stack);

  if (stack_top->value < next_pixel->value)
  {
    mt_stack_insert(&mt->stack, next_pixel);
  }

  stack_top = MT_STACK_TOP(&mt->stack);
  INT_TYPE stack_top_index = MT_INDEX_OF(*stack_top);

  mt->nodes[old_top_index].parent = stack_top_index;
  mt_merge_nodes(mt, stack_top_index, old_top_index);
}

static void mt_remaining_stack(mt_data* mt)
{
  while (MT_STACK_SIZE(&mt->stack) > 1)
  {
    mt_pixel old_top = *mt_stack_remove(&mt->stack);
    INT_TYPE old_top_index = MT_INDEX_OF(old_top);

    mt_pixel* stack_top = MT_STACK_TOP(&mt->stack);
    INT_TYPE stack_top_index = MT_INDEX_OF(*stack_top);

    mt->nodes[old_top_index].parent = stack_top_index;
    mt_merge_nodes(mt, stack_top_index, old_top_index);
  }
}

void mt_flood(mt_data* mt)
{
  assert(mt->connectivity.height > 0);
  assert(mt->connectivity.height % 2 == 1);
  assert(mt->connectivity.width > 0);
  assert(mt->connectivity.width % 2 == 1);

  if (mt->verbosity_level)
  {
    int num_neighbors = 0;
    int i;
    for (i = 0; i != mt->connectivity.height; ++i)
    {
      int j;
      for (j = 0; j != mt->connectivity.width; ++j)
      {
        if (mt->connectivity.neighbors[i * mt->connectivity.width + j])
        {
          ++num_neighbors;
        }
      }
    }

    printf("%d neighbors connectivity.\n", num_neighbors);
  }

  mt_pixel next_pixel = mt_starting_pixel(mt);
  INT_TYPE next_index = MT_INDEX_OF(next_pixel);
  mt->root = mt->nodes + next_index;
  mt->nodes[next_index].parent = MT_NO_PARENT;
  mt_heap_insert(&mt->heap, &next_pixel);
  mt_stack_insert(&mt->stack, &next_pixel);

  while (MT_HEAP_NOT_EMPTY(&mt->heap))
  {
    mt_pixel pixel = next_pixel;
    INT_TYPE index = next_index;

    mt_queue_neighbours(mt, &pixel);

    next_pixel = *MT_HEAP_TOP(&mt->heap);
    next_index = MT_INDEX_OF(next_pixel);

    if (next_pixel.value > pixel.value)
    {
      // Higher level

      mt_stack_insert(&mt->stack, &next_pixel);
      continue;
    }

    pixel = *mt_heap_remove(&mt->heap);
    index = MT_INDEX_OF(pixel);
    mt_pixel *stack_top = MT_STACK_TOP(&mt->stack);
    INT_TYPE stack_top_index = MT_INDEX_OF(*stack_top);

    if (index != stack_top_index)
    {
      mt->nodes[index].parent = stack_top_index;
      ++mt->nodes[stack_top_index].area;
    }

    if (MT_HEAP_EMPTY(&mt->heap))
    {
      break;
    }

    next_pixel = *MT_HEAP_TOP(&mt->heap);
    next_index = MT_INDEX_OF(next_pixel);

    if (next_pixel.value < pixel.value)
    {
      // Lower level

      mt_descend(mt, &next_pixel);
    }
  }

  mt_remaining_stack(mt);

  mt_stack_free_entries(&mt->stack);
  mt_heap_free_entries(&mt->heap);
}

void mt_init(mt_data* mt, const image* img)
{
  mt->img = *img;

  mt->nodes = safe_malloc(mt->img.size * sizeof(mt_node));
  mt->nodes_attributes = safe_calloc(mt->img.size,
    sizeof(mt_node_attributes));

  mt_stack_alloc_entries(&mt->stack);
  mt_heap_alloc_entries(&mt->heap);

  mt_init_nodes(mt);

  mt->connectivity.neighbors = mt_conn_4;
  mt->connectivity.width = MT_CONN_4_WIDTH;
  mt->connectivity.height = MT_CONN_4_HEIGHT;

  mt->verbosity_level = 0;
}

void mt_free(mt_data* mt)
{
  // Free the memory occupied by the max tree
  free(mt->nodes);
  free(mt->nodes_attributes);

  //memset(mt, 0, sizeof(mt_data));
}

void mt_set_verbosity_level(mt_data* mt, int verbosity_level)
{
  mt->verbosity_level = verbosity_level;
}
