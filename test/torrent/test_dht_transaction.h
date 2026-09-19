#include "helpers/test_main_thread.h"

class test_dht_transaction : public TestFixtureWithMainNetTrackerThread {
  CPPUNIT_TEST_SUITE(test_dht_transaction);

  CPPUNIT_TEST(test_key_includes_port);
  CPPUNIT_TEST(test_query_transaction_id_is_two_bytes);
  CPPUNIT_TEST(test_reply_from_other_port_is_not_matched);

  CPPUNIT_TEST_SUITE_END();

public:
  void setUp() override;
  void tearDown() override;

  void test_key_includes_port();
  void test_query_transaction_id_is_two_bytes();
  void test_reply_from_other_port_is_not_matched();
};
