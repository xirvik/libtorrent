#include "config.h"

#include "test/net/test_dns_buffer.h"

#include <atomic>
#include <chrono>
#include <cstdlib>
#include <future>
#include <netinet/in.h>
#include <poll.h>
#include <string>
#include <sys/socket.h>
#include <unistd.h>

#include "torrent/net/socket_address.h"
#include "torrent/system/callbacks.h"

CPPUNIT_TEST_SUITE_NAMED_REGISTRATION(test_dns_buffer, "net");

namespace {

constexpr int extra_queries = 5;

} // namespace

void
test_dns_buffer::setUp() {
  m_nameserver_fd = ::socket(AF_INET, SOCK_DGRAM, 0);
  CPPUNIT_ASSERT(m_nameserver_fd >= 0);

  auto sa = torrent::sa_make_inet_h(0x7f000001, 0);
  CPPUNIT_ASSERT(::bind(m_nameserver_fd, sa.get(), sizeof(sockaddr_in)) == 0);

  sockaddr_in bound     = {};
  socklen_t   sa_length = sizeof(bound);
  CPPUNIT_ASSERT(::getsockname(m_nameserver_fd, reinterpret_cast<sockaddr*>(&bound), &sa_length) == 0);

  // Point the resolver at a socket on the loopback interface that never
  // answers, so submitted queries stay active and none leaves the host.
  ::setenv("NSCACHEIP", "127.0.0.1", 1);
  ::setenv("RES_OPTIONS", ("port:" + std::to_string(ntohs(bound.sin_port))).c_str(), 1);

  TestFixtureWithMainNetTrackerThread::setUp();
}

void
test_dns_buffer::tearDown() {
  TestFixtureWithMainNetTrackerThread::tearDown();

  m_buffer.reset();

  if (m_nameserver_fd >= 0)
    ::close(m_nameserver_fd);

  ::unsetenv("NSCACHEIP");
  ::unsetenv("RES_OPTIONS");
}

void
test_dns_buffer::test_pending_queries_are_capped() {
  auto total = torrent::net::DnsBuffer::max_requests +
    static_cast<int>(torrent::net::DnsBuffer::max_pending_requests) + extra_queries;

  m_buffer = std::make_unique<torrent::net::DnsBuffer>();

  auto              failures = std::make_shared<std::atomic<int>>(0);
  std::promise<int> submitted;

  torrent::net_thread::callback_interrupt([this, total, failures, &submitted]() {
      for (int i = 0; i < total; i++)
        m_buffer->resolve(this, "host" + std::to_string(i) + ".test.invalid", AF_INET,
                          [failures](auto, int error_sin, auto, int) {
                            if (error_sin != 0)
                              (*failures)++;
                          });

      submitted.set_value(failures->load());
    });

  auto result = submitted.get_future();

  CPPUNIT_ASSERT(result.wait_for(std::chrono::seconds(10)) == std::future_status::ready);
  CPPUNIT_ASSERT_EQUAL(extra_queries, result.get());

  // The active queries reached the loopback socket, not a real resolver.
  pollfd descriptor = {m_nameserver_fd, POLLIN, 0};

  CPPUNIT_ASSERT(::poll(&descriptor, 1, 5000) == 1);
}
