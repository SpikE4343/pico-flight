#ifndef __math_fixed_h_INCLUDED__
#define __math_fixed_h_INCLUDED__

#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>

#define FRACTION_BITS 29
#define INTEGER_BITS 3

typedef union
{
  int32_t value;
  struct {
    int32_t fraction:FRACTION_BITS;
    int32_t whole:INTEGER_BITS;
  };
  /* data */
} fixed_t;

typedef union
{
  int64_t value;
  struct {
    int64_t fraction:FRACTION_BITS;
    int64_t whole:INTEGER_BITS;
  };
  /* data */
} fixed64_t;

inline bool fixed_eq(fixed_t a, fixed_t b)
{
  return a.value == b.value;
}

inline fixed_t fixed_mul(fixed_t a, fixed_t b)
{
  fixed_t f = {.value=(int32_t)(((int64_t)a.value*(int64_t)b.value)>>FRACTION_BITS)};
  return f;
}

inline fixed_t fixed_div(fixed_t a, fixed_t b)
{
  fixed_t f = {(int32_t)((((int64_t)a.value) << FRACTION_BITS) / (int64_t)b.value)};
  //fixed_t f = {(int32_t)((((int64_t)a.value) << FRACTION_BITS) / (int64_t)b.value)};
  return f;
}

inline fixed_t fixed_add(fixed_t a, fixed_t b)
{
  fixed_t f = {a.value + b.value};
  return f;
}

inline fixed_t fixed_sub(fixed_t a, fixed_t b)
{
  fixed_t f = {a.value - b.value};
  return f;
}

inline fixed_t fixed_from_int(int32_t a)
{
  fixed_t f = {a << FRACTION_BITS};

  return f;
}

inline int32_t fixed_to_int(fixed_t a)
{
  // TODO: rounding?
  return a.whole;
}

inline fixed_t fixed_from_float(float f)
{
  fixed_t rt = {
   .value = (int32_t)(f * (1<<FRACTION_BITS))
  };

  return rt;
}

inline float fixed_to_float(fixed_t a)
{
  // TODO: rounding?
  return ((float)a.value / (float)(1 << FRACTION_BITS));
}

inline void fixed_print( fixed_t f)
{
  printf("{ v=[%d, %X], w=[%d, %1X], f=[%d,%X] }", 
    f.value,  f.value, 
    f.whole,  (uint8_t)f.whole, 
    f.fraction, f.fraction);
}


inline bool fixed_gt(fixed_t a, fixed_t b)
{
  return a.value > b.value;
}


inline bool fixed_lt(fixed_t a, fixed_t b)
{
  return a.value < b.value;
}

inline bool fixed_gte(fixed_t a, fixed_t b)
{
  return a.value >= b.value;
}


inline bool fixed_lte(fixed_t a, fixed_t b)
{
  return a.value <= b.value;
}

inline fixed_t fixed_min(fixed_t a, fixed_t b)
{
  return a.value > b.value ? b : a;
}

inline fixed_t fixed_max(fixed_t a, fixed_t b)
{
  return a.value > b.value ? a : b;
}

inline fixed_t fixed_rshift(fixed_t a, int num)
{
  fixed_t rt = {
   .value = a.value >> num
  };

  return rt;
}

inline fixed_t fixed_lshift(fixed_t a, int num)
{
  fixed_t rt = {
   .value = a.value << num
  };

  return rt;
}


#define fixed_clamp(m, v, mx) (fixed_max(m, fixed_min(v, mx)))


inline fixed_t fixed_lowpass_filter(fixed_t value, fixed_t sample, fixed_t alpha)
{
  fixed_t accum = fixed_sub(sample, value);
  accum = fixed_mul(alpha, accum);
  accum = fixed_add(value, accum);
  return accum;
}

#endif