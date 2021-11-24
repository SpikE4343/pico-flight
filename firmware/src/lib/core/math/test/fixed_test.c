#include <assert.h>
#include <stdio.h>
#include <stddef.h>
#include <stdbool.h>

#include "basic_test.h"
#include "../math_fixed.h"

int test_add_int()
{
  int ai = 1;
  int bi = 1;
  fixed_t a = fixed_from_int(ai);
  // fixed_print(a);

  fixed_t b = fixed_from_int(bi);
  // fixed_print(b);

  fixed_t r = fixed_add(a, b);
  // fixed_print(r);
  int32_t ir = fixed_to_int(r);

  uint32_t ow =  sizeof(fixed_t);
  uint32_t of =  0;


  // printf("ow: %u, of:%u, ir=%d\n", ow, of, ir);
  assert( ir == ai+bi );
  
  return 1;
}

int test_add_float()
{
  float fa = 0.25f;
  float fb = 0.25f;

  fixed_t a = fixed_from_float(fa);
  // fixed_print(a);
  // printf("%f\n", fixed_to_float(a));

  fixed_t b = fixed_from_float(fb);
  // fixed_print(b);
  // printf("%f\n", fixed_to_float(b));

  fixed_t r = fixed_add(a, b);
  // fixed_print(r);
  int32_t ir = fixed_to_int(r);
  float fr = fixed_to_float(r);

  // printf("ir=%d, fr=%f\n", ir, fr);
  BT_ASSERT( ir == (int32_t)(fa+fb) );
  BT_ASSERT( fr == fa+fb);
  
  return 1;
}

int test_convert_float()
{
  float f = (float)(1000.0/(65535.0/2.0));

  fixed_t ff = fixed_from_float(f);
  // fixed_print(ff);

  float rf = fixed_to_float(ff);
  // printf("%f == %f\n", f, rf);
  BT_ASSERT(rf == f);

  return 1;
}

int test_mul_float()
{
  float fa = -0.25f;
  float fb = 0.25f;

  fixed_t a = fixed_from_float(fa);
  // fixed_print(a);
  // printf("%f\n", fixed_to_float(a));

  fixed_t b = fixed_from_float(fb);
  // fixed_print(b);
  // printf("%f\n", fixed_to_float(b));

  fixed_t r = fixed_mul(a, b);
  // fixed_print(r);
  float fr = fixed_to_float(r);

  BT_ASSERT( fr == fa*fb );
  
  return 1;
}

int test_div_float()
{
  float fa = 1.25f;
  float fb = -0.5f;

  fixed_t a = fixed_from_float(fa);
  // fixed_print(a);
  // printf("%f\n", fixed_to_float(a));

  fixed_t b = fixed_from_float(fb);
  // fixed_print(b);
  // printf("%f\n", fixed_to_float(b));

  fixed_t r = fixed_div(a, b);
  // fixed_print(r);
  float fr = fixed_to_float(r);
  // printf("%f, %f\n", fr, fa/fb);

  BT_ASSERT( fr == fa/fb );
  
  return 1;
}

int test_gyro_convert()
{
  int16_t max = 1600;
  int16_t min = 32768;

  fixed_t a = {((int32_t)max+1) << 14};
  fixed_print(a);
  // printf("%d=%f\n", max, fixed_to_float(a));

  fixed_t b = {((int32_t)min) << 14};
  fixed_print(b);
  
  
  fixed_t one = fixed_from_int(1);
  fixed_t two = fixed_from_int(2);

  fixed_t half = fixed_div(one, two);
  fixed_t pi = fixed_from_float(3.14159f);
  fixed_t tpi = fixed_div(one, pi);
  


  fixed_print(b);

  return 1;
}

int test_min_max()
{
  float a = -1;
  float b = 1;

  fixed_t fa = fixed_from_float(a);
  fixed_t fb = fixed_from_float(b);

  BT_ASSERT(fixed_eq(fa, fa));
  BT_ASSERT(fixed_eq(fixed_min(fa,fb), fa));
  BT_ASSERT(fixed_eq(fixed_max(fa,fb), fb));
  
  return 1;
}

int test_less_than()
{
  float a = -1;
  float b = 1;

  fixed_t fa = fixed_from_float(a);
  fixed_t fb = fixed_from_float(b);

  BT_ASSERT(a < b);
  BT_ASSERT(fixed_lt(fa,fb));
  
  return 1;
}

int test_less_than_equal()
{
  float a = -1;
  float b = 1;

  fixed_t fa = fixed_from_float(a);
  fixed_t fb = fixed_from_float(a);

  BT_ASSERT(a <= b);
  BT_ASSERT(fixed_lte(fa,fb));
  
  return 1;
}

int test_greater_than_equal()
{
  float a = -1;
  float b = 1;

  fixed_t fa = fixed_from_float(a);
  fixed_t fb = fixed_from_float(b);

  BT_ASSERT(b >= a);
  BT_ASSERT(fixed_gte(fb,fa));
  
  return 1;
}

int test_greater_than()
{
  float a = -1;
  float b = 1;

  fixed_t fa = fixed_from_float(a);
  fixed_t fb = fixed_from_float(b);

  BT_ASSERT(b > a);
  BT_ASSERT(fixed_gt(fb,fa));
  
  return 1;
}

BT_SETUP();

int main()
{
  BT_BEGIN();

  BT_ADD_TEST(test_convert_float);
  BT_ADD_TEST(test_add_int);
  BT_ADD_TEST(test_add_float);
  BT_ADD_TEST(test_mul_float);
  BT_ADD_TEST(test_div_float);
  BT_ADD_TEST(test_gyro_convert);
  BT_ADD_TEST(test_min_max);
  BT_ADD_TEST(test_less_than);
  BT_ADD_TEST(test_less_than_equal);
  BT_ADD_TEST(test_greater_than);
  BT_ADD_TEST(test_greater_than_equal);

  BT_END();
}