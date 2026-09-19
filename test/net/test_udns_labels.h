#ifndef LIBTORRENT_TEST_NET_TEST_UDNS_LABELS_H
#define LIBTORRENT_TEST_NET_TEST_UDNS_LABELS_H

#include "helpers/test_fixture.h"

class test_udns_labels : public test_fixture {
  CPPUNIT_TEST_SUITE(test_udns_labels);

  CPPUNIT_TEST(test_reserved_label_type);
  CPPUNIT_TEST(test_compression_pointer);

  CPPUNIT_TEST_SUITE_END();

public:
  void test_reserved_label_type();
  void test_compression_pointer();
};

#endif
