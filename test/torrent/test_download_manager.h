#include "helpers/test_fixture.h"

class test_download_manager : public test_fixture {
  CPPUNIT_TEST_SUITE(test_download_manager);
  CPPUNIT_TEST(test_find_rejects_wrong_length);
  CPPUNIT_TEST_SUITE_END();

public:
  void test_find_rejects_wrong_length();
};
