#include "helpers/test_fixture.h"

class test_initialize : public test_fixture {
  CPPUNIT_TEST_SUITE(test_initialize);

  CPPUNIT_TEST(test_failed_initialize_leaves_library_uninitialized);

  CPPUNIT_TEST_SUITE_END();

public:
  void test_failed_initialize_leaves_library_uninitialized();
};
