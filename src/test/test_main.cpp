#include "test_main.h"

#include <halt.h>
#include <test.h>

int test_get_content(int argc, char *argv[]) {
  TESTS();

  ADD_UNIT(TestExample);
  ADD_UNIT(TestEmbList);

  RUN_ALL_TESTS();
  halt();
  return 0;
}
