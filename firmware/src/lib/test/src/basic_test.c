#include "basic_test.h"


int basic_test_run_test(test_function_t func, char* name)
{
  ++basic_test_data()->total;
  //printf("--------------------------\n");
  
  int r = func();

  if(r)
  {
    ++basic_test_data()->passing;
    printf("[%sPASS%s] %s\n", KGRN, KNRM, name);
  }
  else
  {
    ++basic_test_data()->failing;
  }

  return r;
}

int basic_test_results()
{
  printf("--------------------------------------------------\n");
  printf("[%s%u%s:Failing, %s%u%s:Passing, %u:Total]\n\n", 
    basic_test_data()->failing ? KRED : KNRM, basic_test_data()->failing, KNRM, 
    KGRN, basic_test_data()->passing, KNRM, 
    basic_test_data()->total);

  return basic_test_data()->failing;
}