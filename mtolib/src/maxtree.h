#ifndef MT_H
#define MT_H

//#include "main.h"

#define MT_UNASSIGNED -1
#define MT_IN_QUEUE -2
#define MT_NO_PARENT -3

#define MT_NO_OBJECT -1

#define MT_IS_ROOT(MT_PTR, IDX) ((MT_PTR)->nodes + IDX == \
  (MT_PTR)->root)

#define MT_CONN_12_WIDTH 5
#define MT_CONN_12_HEIGHT 5

#define MT_CONN_8_WIDTH 3
#define MT_CONN_8_HEIGHT 3

#define MT_CONN_4_WIDTH 3
#define MT_CONN_4_HEIGHT 3

extern const int mt_conn_12[MT_CONN_12_HEIGHT * MT_CONN_12_WIDTH];
extern const int mt_conn_8[MT_CONN_8_HEIGHT * MT_CONN_8_WIDTH];
extern const int mt_conn_4[MT_CONN_4_HEIGHT * MT_CONN_4_WIDTH];

typedef struct
{
  INT_TYPE parent;
  INT_TYPE area;
} mt_node;

typedef struct
{
  FLOAT_TYPE volume;
  FLOAT_TYPE power;
} mt_node_attributes;

typedef struct
{
  SHORT_TYPE x;
  SHORT_TYPE y;
} mt_image_location;

typedef struct
{
  mt_image_location location;
  PIXEL_TYPE value;
} mt_pixel;

#include "mt_heap.h"
#include "mt_stack.h"

typedef struct
{
  const int* neighbors;
  int height;
  int width;
} mt_connectivity;

typedef struct
{
  mt_node *root;
  mt_node *nodes;
  mt_node_attributes *nodes_attributes;
  mt_heap heap;
  mt_stack stack;
  image img;
  mt_connectivity connectivity;  
  int verbosity_level;
} mt_data;

void* mt_malloc(size_t size);

void mt_flood(mt_data* mt);
void mt_init(mt_data* mt, const image* img);
void mt_free(mt_data* mt);

void mt_set_verbosity_level(mt_data* mt, int verbosity_level);

#endif
