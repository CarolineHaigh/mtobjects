#ifndef MAIN_H_3947FB9C
#define MAIN_H_3947FB9C

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

/* Settings */

#define BG_REJECTION_RATE 0.05

#define BG_TILE_SIZE_START 64
#define BG_TILE_SIZE_MIN 16
#define BG_TILE_SIZE_MAX 128

/* End of settings */

#define INT_TYPE   int32_t
#define SHORT_TYPE int16_t 

#define FLOAT_TYPE float

#define PIXEL_TYPE float
#define FITS_TYPE TFLOAT

#define TRUE  1
#define FALSE 0

typedef struct
{
  PIXEL_TYPE* data;
  SHORT_TYPE height;
  SHORT_TYPE width;
  INT_TYPE size;
} image;

static inline void image_set(image* img, PIXEL_TYPE *data,
  SHORT_TYPE height, SHORT_TYPE width)
{
  img->data = data;
  img->height = height;
  img->width = width;
  img->size = height * width;
}

static inline void image_free(image* img)
{
  free(img->data);
}

static inline void error(const char* format, ...)
{
  va_list arg_ptr;
  va_start(arg_ptr, format);
  vfprintf(stderr, format, arg_ptr);
  va_end(arg_ptr);
  
  exit(EXIT_FAILURE);
}

static inline void* safe_malloc(size_t size)
{
  void *ptr = malloc(size);
    
  if (ptr == NULL)
  {
    error("malloc(%d) failed.\n", size);
  }
  
  return ptr;
}

static inline void* safe_calloc(size_t num_elem, size_t size)
{
  void *ptr = calloc(num_elem, size);
    
  if (ptr == NULL)
  {
    error("calloc(%d, %d) failed.\n", num_elem, size);
  }
  
  return ptr;
}

static inline void* safe_realloc(void *ptr, size_t size)
{
  ptr = realloc(ptr, size);
    
  if (ptr == NULL)
  {
    error("realloc(ptr, %d) failed.\n", size);
  }
  
  return ptr;
}

#endif
