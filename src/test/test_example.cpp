#include <test.h>

/*
   How to write tests:

   1. Add this object file to the TEST_DEPS in the makefile.

   2. Come up with a unit name. No underscores. E.g. TestExample

   3. Call ADD_UNIT(<unit_name>) in test_main.cpp, e.g. ADD_UNIT(TestExample)

   4. Declare UNIT(<unit_name>), as in this file.

   5. Declare your collection of TEST(<unit_name>, <test_name>) in this file.

   6. Add each TEST to UNIT using ADD_TEST.

   No need to make any header files or declarations.

   EXAMPLE:

 */

UNIT(TestExample) {
  ADD_TEST(TestExample, CommonCase1);
  ADD_TEST(TestExample, CommonCase2);
  ADD_TEST(TestExample, EdgeCase1);
  ADD_TEST(TestExample, EdgeCase2);
}

TEST(TestExample, CommonCase1) { ASSERT_EQ(2, 1 + 1); }

TEST(TestExample, CommonCase2) { ASSERT_TRUE(1 == 1); }

TEST(TestExample, EdgeCase1) {
  ASSERT_EQ(reinterpret_cast<void *>(0), nullptr);
}

TEST(TestExample, EdgeCase2) { ASSERT_TRUE(1l + 0xffffffff == 0x100000000); }
