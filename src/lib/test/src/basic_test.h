#ifndef __BASIC_TEST_INCLUDED__
#define __BASIC_TEST_INCLUDED__

#include <stdio.h>
#include <stdint.h>

// https://stackoverflow.com/questions/5412761/using-colors-with-printf
#define KNRM  "\x1B[0m"
#define KRED  "\x1B[31m"
#define KGRN  "\x1B[32m"
#define KYEL  "\x1B[33m"
#define KBLU  "\x1B[34m"
#define KMAG  "\x1B[35m"
#define KCYN  "\x1B[36m"
#define KWHT  "\x1B[37m"

typedef struct {
  uint32_t total;
  uint32_t passing;
  uint32_t failing;
} BasicTest_data_t;

BasicTest_data_t* basic_test_data();

#define BT_SETUP() BasicTest_data_t __test_data = { 0, 0, 0}; \
BasicTest_data_t* basic_test_data() { return &__test_data; }

// always at start of test runner function ie: main()
#define BT_BEGIN() printf("--------------------------------------------------\n");

#define BT_ASSERT(expr){ if(!(expr)) { printf("[%sFAIL%s] \"%s\" at %s:%d\n", KRED, KNRM, #expr, __FILE__, __LINE__); return 0; }}

#define BT_ASSERT_MSG(expr, fail_msg){ if(!(expr)) { printf("[%sFAILED%s] %s \"%s\" at %s:%d\n", KRED, KNRM, fail_msg, #expr, __FILE__, __LINE__); return 0; }}

#define BT_FAIL(expr){ printf("[%sFAIL%s] \"%s\" at %s:%d\n", KRED, KNRM, #expr, __FILE__, __LINE__); return 0; }

typedef int (*test_function_t)();

int basic_test_run_test(test_function_t func, char* name);
int basic_test_results();

#define BT_ADD_TEST(func) basic_test_run_test(func, #func);

#define BT_END() return basic_test_results();

// int test_add_int()
// {
//   int ai = 1;
//   int bi = 1;
//   fixed_t a = fixed_from_int(ai);
//   fixed_print(a);

//   fixed_t b = fixed_from_int(bi);
//   fixed_print(b);

//   fixed_t r = fixed_add(a, b);
//   fixed_print(r);
//   int32_t ir = fixed_to_int(r);

//   uint32_t ow =  sizeof(fixed_t);
//   uint32_t of =  0;


//   printf("ow: %u, of:%u, ir=%d\n", ow, of, ir);
//   assert( ir == ai+bi );
  
//   return 1;
// }

// int test_add_float()
// {
//   float fa = 0.25f;
//   float fb = 0.25f;

//   fixed_t a = fixed_from_float(fa);
//   fixed_print(a);
//   printf("%f\n", fixed_to_float(a));

//   fixed_t b = fixed_from_float(fb);
//   fixed_print(b);
//   printf("%f\n", fixed_to_float(b));

//   fixed_t r = fixed_add(a, b);
//   fixed_print(r);
//   int32_t ir = fixed_to_int(r);
//   float fr = fixed_to_float(r);


//   printf("ir=%d, fr=%f\n", ir, fr);
//   ASSERT( ir == (int32_t)(fa+fb) );
//   ASSERT( fr != fa+fb);
  
//   return 1;
// }

// int test_convert_float()
// {
//   float f = (float)(1000.0/(65535.0/2.0));

//   fixed_t ff = fixed_from_float(f);
//   fixed_print(ff);

//   float rf = fixed_to_float(ff);
//   printf("%f == %f\n", f, rf);
//   ASSERT(rf == f);

//   return 1;
// }



// int main()
// {
//   ADD_TEST(test_convert_float);
//   ADD_TEST(test_add_int);
//   ADD_TEST(test_add_float);

//   TEST_RESULTS();
// }

#endif