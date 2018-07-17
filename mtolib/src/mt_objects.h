#include "maxtree.h"

#ifndef MT_OBJECTS_H
#define MT_OBJECTS_H

#define MT_SET_SIGNIFICANT(IDX) (mt_o->flags[IDX] |= 1)
#define MT_SIGNIFICANT(IDX) (mt_o->flags[IDX] & 1)

#define MT_SET_CHECKED_FOR_SIGNIFICANT_ANCESTOR(IDX) \
  (mt_o->flags[IDX] |= 2)
#define MT_CHECKED_FOR_SIGNIFICANT_ANCESTOR(IDX) (mt_o->flags[IDX] & 2)

#define MT_SET_HAVE_SIGNIFICANT_DESCENDANT(IDX) (mt_o->flags[IDX] |= 4)
#define MT_HAVE_SIGNIFICANT_DESCENDANT(IDX) (mt_o->flags[IDX] & 4)

#define MT_UNSET_OBJECT(IDX) (mt_o->flags[IDX] &= ~8)
#define MT_SET_OBJECT(IDX) (mt_o->flags[IDX] |= 8)
#define MT_OBJECT(IDX) (mt_o->flags[IDX] & 8)

#define MT_SET_HAVE_DESCENDANT(IDX) (mt_o->flags[IDX] |= 16)
#define MT_HAVE_DESCENDANT(IDX) (mt_o->flags[IDX] & 16)

#define MT_SET_DONT_MOVE(IDX) (mt_o->flags[IDX] |= 32)
#define MT_DONT_MOVE(IDX) (mt_o->flags[IDX] & 32)

#define MT_SET_CHECKED_FOR_OBJECT(IDX) (mt_o->flags[IDX] |= 64)
#define MT_CHECKED_FOR_OBJECT(IDX) (mt_o->flags[IDX] & 64)

#define MT_HAVE_SIGNIFICANT_ANCESTOR(IDX) \
  (mt_o->closest_significant_ancestors[IDX] != MT_NO_PARENT)
  
#define MT_DISTANCE(IDX) \
  (MT_HAVE_SIGNIFICANT_ANCESTOR(IDX) ? \
    mt->img.data[IDX] - \
      mt->img.data[mt_o->closest_significant_ancestors[IDX]] : \
    mt->img.data[IDX])      

struct mt_object_data;
struct parameters;

typedef struct mt_parameters
{
  INT_TYPE verbosity_level;
  double bg_variance;
  double gain;
  double move_factor;
  double alpha;
  double min_distance;
}mt_parameters;

typedef struct mt_object_data
{  
  mt_data* mt;
  mt_parameters* paras;
  uint8_t* flags;
  INT_TYPE* relevant_indices;
  INT_TYPE relevant_indices_len;
  INT_TYPE* closest_significant_ancestors;
  INT_TYPE* main_branches;
  INT_TYPE* main_power_branches;
  INT_TYPE* object_ids;
  INT_TYPE num_significant_nodes;
  INT_TYPE num_objects;
  // Pointer to function that takes an mt_o and a node ID, and returns an int
  int (*node_significance_test)(struct mt_object_data *, INT_TYPE);
  // Pointer to function that takes an mt_o and returns nothing
  void (*significant_nodes)(struct mt_object_data *);
  void *node_significance_test_data;
  void (*node_significance_test_data_free)(struct mt_object_data *);
} mt_object_data;

void mt_objects_init(mt_object_data* mt_o);
void mt_objects_free(mt_object_data* mt_o);
void mt_ids_free(mt_object_data* mt_o);
void mt_objects(mt_object_data* mt_o);
void mt_object_ids(mt_object_data* mt_o);

void mt_relevant_nodes(mt_object_data* mt_o);

void mt_use_node_test_4(mt_object_data* mt_o);

void node_significance_test_data_clear(mt_object_data* mt_o);

#define MT_NO_MAX_DISTANCE -1.0

FLOAT_TYPE mt_noise_variance(mt_object_data* mt_o,
  INT_TYPE node_idx, FLOAT_TYPE max_normalized_distance);
FLOAT_TYPE mt_alternative_power_definition(mt_object_data* mt_o,
  INT_TYPE node_idx, FLOAT_TYPE max_normalized_distance);

#endif
