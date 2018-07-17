#include <math.h>
#include <gsl/gsl_cdf.h>
#include <assert.h>

#include "maxtree.h"
#include "mt_objects.h"
//#include "mt_heap.h"



void mt_relevant_nodes(mt_object_data* mt_o)
{
  // Sort the nodes by image value.
  // Do not include the root or nodes where the parent has the same image value.

  mt_data *mt = mt_o->mt;

  // Create a heap
  mt_heap heap;
  mt_heap_alloc_entries(&heap);
  
  // Iterate over image pixels
  SHORT_TYPE y;
  for (y = 0; y != mt->img.height; ++y)
  {
    SHORT_TYPE x;
    for (x = 0; x != mt->img.width; ++x)
    {

      // Get the pixel index and parent index
      INT_TYPE i = y * mt->img.width + x;
      INT_TYPE parent_idx = mt->nodes[i].parent;

      // Skip the root node
      // Skip nodes where the parent is at the same level as the node
      if (MT_IS_ROOT(mt, i) ||
        mt->img.data[parent_idx] == mt->img.data[i])
      {
        //printf("Root: %i\n", i);
        continue;
      }

      // Create a pixel object and put it on the heap
      mt_pixel pixel;
      pixel.location.x = x;
      pixel.location.y = y;
      pixel.value = mt->img.data[i];
      mt_heap_insert(&heap, &pixel);
    }
  }

  // Get the size of the heap and allocate an array of the same length
  // Relevant indices stores level roots
  mt_o->relevant_indices_len = MT_HEAP_SIZE(&heap);
  mt_o->relevant_indices = malloc(MT_HEAP_SIZE(&heap) *
    sizeof(*mt_o->relevant_indices));
 

  // Print number of relevant nodes
  if (mt->verbosity_level > 1)
  {
    printf("Number of nodes to be tested: %d.\n", 
      mt_o->relevant_indices_len);
      
  }

  // Move nodes from heap into relevant indices list
  // Produces list of root node indices sorted by pixel value
  INT_TYPE i;
  for (i = mt_o->relevant_indices_len; i--;)
  {
    const mt_pixel* removed = mt_heap_remove(&heap);
    mt_o->relevant_indices[i] = removed->location.x +
      removed->location.y * mt->img.width;
  }

  // Free heap memory
  mt_heap_free_entries(&heap);
}

void mt_update_parent_main_branch(
  mt_object_data* mt_o, INT_TYPE node_idx)
{
  // Update the ancestor's main branch
  // I.E. the largest child node of this node's ancestor

  mt_data *mt = mt_o->mt;

  // Skip if no significant ancestor
  if (!MT_HAVE_SIGNIFICANT_ANCESTOR(node_idx))
    return;

  // Get the closest significant ancestor of this node
  INT_TYPE ancestor_idx = mt_o->
    closest_significant_ancestors[node_idx];

  // If the ancestor has a significant descendant, check if this node is larger
  // If so, set this node as the ancestor's significant descendant
  if (MT_HAVE_SIGNIFICANT_DESCENDANT(ancestor_idx))
  {
    if (mt->nodes[mt_o->main_branches[ancestor_idx]].area <
      mt->nodes[node_idx].area)
    {
      mt_o->main_branches[ancestor_idx] = node_idx;
    }
  }
  else
  // If the ancestor has no significant descendant, set this node as it
  {
    MT_SET_HAVE_SIGNIFICANT_DESCENDANT(ancestor_idx);
    mt_o->main_branches[ancestor_idx] = node_idx;
  }
}

void mt_significant_nodes_up(mt_object_data* mt_o)
{
  // Test each relevant node for significance

  // Get pointer to data
  mt_data *mt = mt_o->mt;
  
  // Count number of significant objects
  INT_TYPE num_significant = 0;

  // Iterate through objects
  INT_TYPE i;
  for (i = 0; i != mt_o->relevant_indices_len; ++i)
  {
	// Get ith object and its parent
    INT_TYPE node_idx = mt_o->relevant_indices[i];
    INT_TYPE parent_idx = mt->nodes[node_idx].parent;
    
    // If parent is significant, set closest significant ancestor to parent
    if (MT_SIGNIFICANT(parent_idx))
    {
      mt_o->closest_significant_ancestors[node_idx] = parent_idx;
    }
    // Else if the parent has a significant ancestor, set CSA to them
    else if (MT_HAVE_SIGNIFICANT_ANCESTOR(parent_idx))
    {
      mt_o->closest_significant_ancestors[node_idx] =
        mt_o->closest_significant_ancestors[parent_idx];
    }
    
    // TEST THE OBJECT - if significant, set as such and add to count
    if (mt_o->node_significance_test(mt_o, node_idx))
    {
      MT_SET_SIGNIFICANT(node_idx);
      
      ++num_significant;
      
      mt_update_parent_main_branch(mt_o, node_idx);
    }
  }
  
  if (mt->verbosity_level > 1)
  {
    printf("%d significant nodes.\n", num_significant);
  }
  
  mt_o->num_significant_nodes = num_significant;
}

void mt_significant_nodes_down(mt_object_data* mt_o)
{
  // Test each relevant node for significance

  // Get pointer to data
  mt_data *mt = mt_o->mt;

  // Count number of significant objects
  INT_TYPE num_significant = 0;

  // Iterate through objects
  INT_TYPE i;
  for (i = mt_o->relevant_indices_len -1 ; i < 0 ; i--)
  {
	// Get ith object and its parent
    INT_TYPE node_idx = mt_o->relevant_indices[i];
    INT_TYPE parent_idx = mt->nodes[node_idx].parent;

    // If parent is significant, set closest significant ancestor to parent
    if (MT_SIGNIFICANT(parent_idx))
    {
      mt_o->closest_significant_ancestors[node_idx] = parent_idx;
    }
    // Else if the parent has a significant ancestor, set CSA to them
    else if (MT_HAVE_SIGNIFICANT_ANCESTOR(parent_idx))
    {
      mt_o->closest_significant_ancestors[node_idx] =
        mt_o->closest_significant_ancestors[parent_idx];
    }

    // TEST THE OBJECT - if significant, set as such and add to count
    if (mt_o->node_significance_test(mt_o, node_idx))
    {
      MT_SET_SIGNIFICANT(node_idx);

      ++num_significant;

      mt_update_parent_main_branch(mt_o, node_idx);
    }
  }

  if (mt->verbosity_level)
  {
    printf("%d significant nodes.\n", num_significant);
  }

  mt_o->num_significant_nodes = num_significant;
}

void mt_find_objects(mt_object_data* mt_o)
{
  // Count significant nodes and set object markers

  mt_data *mt = mt_o->mt;

  INT_TYPE num_objects = 0;
  INT_TYPE num_objects_nested = 0;
  
  INT_TYPE i;

  // Iterate over pixels in image
  for (i = 0; i != mt->img.size; ++i)  
  {
    // Skip if no significant flag
    if (!MT_SIGNIFICANT(i))
    {
      continue;
    }

    // Count and mark as object if no significant ancestor
    if (!MT_HAVE_SIGNIFICANT_ANCESTOR(i))
    {
      ++num_objects;
      MT_SET_OBJECT(i);

      continue;
    }

    // If it has a significant ancestor, and that ancestor's largest descendant is NOT this node,
        // mark it as a nested object
    INT_TYPE parent = mt_o->closest_significant_ancestors[i];

    if (mt_o->main_branches[parent] != i)
    {
      ++num_objects_nested;
      MT_SET_OBJECT(i);
      continue;
    }

    // i.e. significant nodes who ARE the significant descendant of their significant ancestor
    // are not marked as objects
  }

  // Get the total number of objects and print
  num_objects += num_objects_nested;

  if (mt->verbosity_level)
  {
    printf("Found %d objects (including %d nested).\n", num_objects,
      num_objects_nested);
  }
  
  mt_o->num_objects = num_objects;
}

void mt_main_power_branches(mt_object_data* mt_o)
{
  // Find the descendant of each node with the highest power

  mt_data *mt = mt_o->mt;
  
  INT_TYPE i;
  // Iterate over image pixels
  for (i = 0; i != mt->img.size; ++i)  
  {
    // Skip the root
    if (MT_IS_ROOT(mt, i))
    {
      continue;
    }

    // Get the pixel's parent
    INT_TYPE parent = mt->nodes[i].parent;

    // If the parent has a descendant, check if this node has a higher power and set accordingly
    if (MT_HAVE_DESCENDANT(parent))
    {
      if (mt->nodes_attributes[mt_o->main_power_branches[parent]].power
			< mt->nodes_attributes[i].power)
      {
        mt_o->main_power_branches[parent] = i;
      }
    }
    else
    // If the parent has no marked descendant, set this as the highest power descendant
    {
      MT_SET_HAVE_DESCENDANT(parent);
      mt_o->main_power_branches[parent] = i;
    }
  }
}

void mt_move_up(mt_object_data* mt_o)
{
  // Move object markers up the tree

  mt_data *mt = mt_o->mt;

  // Skip this function if no move factor is specified
  if (mt_o->paras->move_factor == 0.0)
  {
    return;
  }

  // Iterate over image pixels
  INT_TYPE i;  
  for (i = 0; i != mt->img.size; ++i)  
  {  
	// Skip if the node is not an object or is marked as 'don't move'
    if (!MT_OBJECT(i) || MT_DONT_MOVE(i))
    {
      continue;
    }
    
    // Mark as not an object
    MT_UNSET_OBJECT(i);
    
    // Base = pixel value - distance from nearest significant ancestor
    FLOAT_TYPE base = mt->img.data[i] - MT_DISTANCE(i);
      
    // scale by gain, background variance, move_factor
    // seems a tad hacky
    base += mt_o->paras->move_factor *
      sqrt(base / mt_o->paras->gain + mt_o->paras->bg_variance);
            
    INT_TYPE next_idx = i;
    // Find next id - first pixel with value less than base or no descendants
    // Check pixels by descending through (significant) descendants
    while (mt->img.data[next_idx] < base)
    {      
    
      if (MT_HAVE_SIGNIFICANT_DESCENDANT(next_idx))
      {
        next_idx = mt_o->main_branches[next_idx];        
      }
      else if (MT_HAVE_DESCENDANT(next_idx))
      {
        next_idx = mt_o->main_power_branches[next_idx];
      }
      else
      {
        break;
      }
    }
    
    // Mark the next ID as an object that cannot be moved    
    MT_SET_OBJECT(next_idx);
    MT_SET_DONT_MOVE(next_idx);
  }
}


void mt_objects_free(mt_object_data* mt_o)
{
  // Free memory from internal arrays
  free(mt_o->flags);
  free(mt_o->main_branches);
  free(mt_o->main_power_branches);
  free(mt_o->relevant_indices);  

  if (mt_o->node_significance_test_data_free != NULL)
  {
    mt_o->node_significance_test_data_free(mt_o);
  }

}


void mt_objects_init(mt_object_data *mt_o)
{
  INT_TYPE img_size = mt_o->mt->img.size;

  mt_o->flags = safe_calloc(img_size, sizeof(*mt_o->flags));

  mt_o->main_branches = safe_malloc(img_size *
    sizeof(*mt_o->main_branches));

  mt_o->main_power_branches = safe_malloc(img_size *
    sizeof(*mt_o->main_power_branches));
}


// The main method for object detection
void mt_objects(mt_object_data* mt_o)
{
  // Create arrays
  mt_objects_init(mt_o);

  // Validate parameters
  assert(mt_o->paras->bg_variance > 0);
  assert(mt_o->paras->gain > 0);
  assert(mt_o->paras->move_factor >= 0);
  assert(mt_o->node_significance_test != NULL);

  // Find level roots
  mt_relevant_nodes(mt_o);

  mt_o->significant_nodes(mt_o);

  // Count objects
  mt_find_objects(mt_o);

  // If move_up is being used, find main power branches and execute move_up
  if(mt_o->paras->move_factor != 0)
  {
    mt_main_power_branches(mt_o);
    mt_move_up(mt_o);
  }

  mt_object_ids(mt_o);
    
}

// Marks object ids
void mt_object_ids(mt_object_data* mt_o)
{
  // Get mt data
  mt_data *mt = mt_o->mt;

  INT_TYPE i;
  // Iterate over image
  for (i = 0; i != mt->img.size; ++i)
  {
	// Skip point if already checked
    if (MT_CHECKED_FOR_OBJECT(i))
    {
      continue;
    }

    // Set next id as i
    INT_TYPE next_idx = i;

    // While next id is an unchecked non-object with a parent
    while (next_idx != MT_NO_PARENT && !MT_OBJECT(next_idx) &&
        !MT_CHECKED_FOR_OBJECT(next_idx))
    {
	  // Set checked for next object, set next object to object's parent
      MT_SET_CHECKED_FOR_OBJECT(next_idx);
      next_idx = mt->nodes[next_idx].parent;
    }

    INT_TYPE object_id;
    INT_TYPE end_idx;

    // If a node without a parent is reached without finding an object, object id = -1 (no object)
    // If an object is found, object id = this node's id
    // If a node that has already been checked is found, object id = that node's object id
    if (next_idx == MT_NO_PARENT)
    {
      object_id = MT_NO_OBJECT;
      end_idx = next_idx;

    }
    else if (MT_CHECKED_FOR_OBJECT(next_idx))
    {
      object_id = mt_o->object_ids[next_idx];
      end_idx = next_idx;

    }
    else if (MT_OBJECT(next_idx))
    {
      object_id = next_idx;

      end_idx = mt->nodes[next_idx].parent;
      MT_SET_CHECKED_FOR_OBJECT(object_id);

    }

    next_idx = i;
    do
    {
      // Mark every id up to the last id examined with the object ID
      mt_o->object_ids[next_idx] = object_id;
      next_idx = mt->nodes[next_idx].parent;

    } while (next_idx != end_idx);
  }
}


void node_significance_test_data_clear(mt_object_data* mt_o)
{
  if (mt_o->node_significance_test_data != NULL)
  {
    mt_o->node_significance_test_data_free(mt_o);
  }
  
  mt_o->node_significance_test_data_free = NULL;
  mt_o->node_significance_test_data = NULL;  
}


FLOAT_TYPE mt_noise_variance(mt_object_data* mt_o,
  INT_TYPE node_idx, FLOAT_TYPE max_normalized_distance)
{
  mt_data* mt = mt_o->mt;

  assert(mt_o->relevant_indices_len > 0); 
  
  FLOAT_TYPE variance = mt_o->paras->bg_variance;

  if (MT_HAVE_SIGNIFICANT_ANCESTOR(node_idx))
  {
    variance +=
      mt->img.data[mt_o->closest_significant_ancestors[node_idx]] /
      mt_o->paras->gain;
  }

  
  if (max_normalized_distance >= 0)
  {
    FLOAT_TYPE distance = MT_DISTANCE(node_idx);
    
    FLOAT_TYPE rms = sqrt(variance);
    
    if (distance / rms > max_normalized_distance)
    {
      FLOAT_TYPE max_normalized_distance_sqr = max_normalized_distance *
        max_normalized_distance;
        
      FLOAT_TYPE gain_sqr = mt_o->paras->gain * mt_o->paras->gain;
      
      FLOAT_TYPE b = 2 * mt->img.data[node_idx] * mt_o->paras->gain;
        
      FLOAT_TYPE f_a = b + max_normalized_distance_sqr - max_normalized_distance *
          sqrt(4 * mt_o->paras->bg_variance * gain_sqr + 2 * b + max_normalized_distance_sqr);
          
      f_a /= 2 * mt_o->paras->gain;
      
      variance = f_a / mt_o->paras->gain + mt_o->paras->bg_variance;
    }
  }
  
  return variance;
}


FLOAT_TYPE mt_alternative_power_definition(mt_object_data* mt_o,
  INT_TYPE node_idx, FLOAT_TYPE max_normalized_distance)
{
  mt_data* mt = mt_o->mt;
  
  assert(mt_o->relevant_indices_len > 0); 
  
  mt_node *node = mt->nodes + node_idx ;
  mt_node_attributes *attr = mt->nodes_attributes + node_idx; 
  
  INT_TYPE parent_idx = node->parent;  
  
  // added distance for the power calculation.
  FLOAT_TYPE delta;
  if (MT_HAVE_SIGNIFICANT_ANCESTOR(node_idx))
  {
    delta = mt->img.data[parent_idx] -
      mt->img.data[mt_o->closest_significant_ancestors[node_idx]];      
  }
  else
  {
    delta = mt->img.data[parent_idx];    
  }  
      
  if (max_normalized_distance >= 0)
  {
    FLOAT_TYPE rms = sqrt(mt_noise_variance(mt_o,
      node_idx, max_normalized_distance));
    
    FLOAT_TYPE distance = MT_DISTANCE(node_idx);

    //printf("%f\n", distance / rms);

    if (distance / rms > max_normalized_distance)
    {
      delta = max_normalized_distance * rms - mt->img.data[parent_idx];
    }  
  }
    
  return attr->power + delta * (2 * attr->volume + delta * node->area);
}
