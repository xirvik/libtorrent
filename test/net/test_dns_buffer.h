#ifndef LIBTORRENT_TEST_NET_TEST_DNS_BUFFER_H
#define LIBTORRENT_TEST_NET_TEST_DNS_BUFFER_H

#include <memory>

#include "helpers/test_main_thread.h"
#include "net/dns_buffer.h"

class test_dns_buffer : public TestFixtureWithMainNetTrackerThread {
  CPPUNIT_TEST_SUITE(test_dns_buffer);

  CPPUNIT_TEST(test_pending_queries_are_capped);

  CPPUNIT_TEST_SUITE_END();

public:
  void setUp() override;
  void tearDown() override;

  void test_pending_queries_are_capped();

private:
  int                                      m_nameserver_fd{-1};
  std::unique_ptr<torrent::net::DnsBuffer> m_buffer;
};

#endif
