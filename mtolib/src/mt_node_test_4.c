#include "mt_objects.h"

#include <assert.h>
#include <math.h>

static const INT_TYPE max_area = 4087;
//static const INT_TYPE max_area = 2;

static const FLOAT_TYPE p1 = 1.683355084690155e-01;
static const FLOAT_TYPE p2 = 3.770229379757511e+02;
static const FLOAT_TYPE p3 = 1.176722049258011e+05;
static const FLOAT_TYPE p4 = 6.239836661965291e+06;
static const FLOAT_TYPE q1 = 1.354265276841128e+03;
static const FLOAT_TYPE q2 = 2.091126298053044e+05;
static const FLOAT_TYPE q3 = 1.424803575269314e+06;


int mt_node_test_4(mt_object_data* mt_o, INT_TYPE node_idx)
{
  // Point to max tree data
  mt_data* mt = mt_o->mt;

  FLOAT_TYPE variance =
    mt_noise_variance(mt_o, node_idx, MT_NO_MAX_DISTANCE);
    
  FLOAT_TYPE min_distance =
    *((FLOAT_TYPE *)mt_o->node_significance_test_data);
    
  if (min_distance > 0 &&
    MT_DISTANCE(node_idx) / sqrt(variance) < min_distance)
  {
    return 0;
  }
  
  FLOAT_TYPE power = mt_alternative_power_definition(mt_o, node_idx,
    MT_NO_MAX_DISTANCE);
    
  INT_TYPE area = mt->nodes[node_idx].area;
    
  FLOAT_TYPE power_normalized = power / variance / area;
  
  if (area > max_area)
  {
    area = max_area;
  }  

  FLOAT_TYPE area_to_2 = area * area;
  FLOAT_TYPE area_to_3 = area_to_2 * area;
  
  FLOAT_TYPE x = p1 * area_to_3 + p2 * area_to_2 + p3 * area + p4;
  x /= area_to_3 + q1 * area_to_2 + q2 * area + q3;

  return power_normalized > x;
}

void mt_node_test_4_data_free(mt_object_data* mt_o)
{
  free(mt_o->node_significance_test_data);
  
  mt_o->node_significance_test_data = NULL;
}

void mt_use_node_test_4(mt_object_data* mt_o)
{
  if (mt_o->mt->verbosity_level)
  {
    printf("Using significance test 4 (powerAlt given area,"
      " 3x3 Gaussian filter with FWHM = 2).\n");
  }

  if (mt_o->paras->alpha != 1e-6)
  {
    error("Error: rejection boundary only available "
      "for alpha = 1E-6.\n");
  }

  node_significance_test_data_clear(mt_o);

  mt_o->node_significance_test_data = malloc(sizeof(FLOAT_TYPE));

  *((FLOAT_TYPE *)mt_o->node_significance_test_data) =
    mt_o->paras->min_distance;

  mt_o->node_significance_test =
    mt_node_test_4;

  mt_o->node_significance_test_data_free =
    mt_node_test_4_data_free;

}
