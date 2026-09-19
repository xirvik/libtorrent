#include "helpers/test_main_thread.h"

class test_network_config : public TestFixtureWithMainThread {
  CPPUNIT_TEST_SUITE(test_network_config);

  CPPUNIT_TEST(test_block_private_peers_defaults_to_off);
  CPPUNIT_TEST(test_block_private_peers_can_be_enabled);

  CPPUNIT_TEST_SUITE_END();

public:
  void test_block_private_peers_defaults_to_off();
  void test_block_private_peers_can_be_enabled();
};
