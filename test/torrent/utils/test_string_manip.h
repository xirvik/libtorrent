#include "helpers/test_fixture.h"

class test_string_manip : public test_fixture {
  CPPUNIT_TEST_SUITE(test_string_manip);
  CPPUNIT_TEST(test_hex_error_sentinel);
  CPPUNIT_TEST(test_from_hex_rejects_invalid);
  CPPUNIT_TEST_SUITE_END();

public:
  void test_hex_error_sentinel();
  void test_from_hex_rejects_invalid();
};
