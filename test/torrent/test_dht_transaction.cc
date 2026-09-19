#include "config.h"

#include "test_dht_transaction.h"

#include <cstdint>
#include <netinet/in.h>
#include <poll.h>
#include <string>
#include <sys/socket.h>
#include <unistd.h>
#include <vector>

#include "dht/dht_transaction.h"
#include "torrent/hash_string.h"
#include "torrent/net/socket_address.h"
#include "torrent/object.h"
#include "torrent/object_stream.h"
#include "torrent/runtime/network_config.h"
#include "torrent/runtime/network_manager.h"
#include "torrent/runtime/socket_manager.h"
#include "torrent/tracker/dht_controller.h"

CPPUNIT_TEST_SUITE_NAMED_REGISTRATION(test_dht_transaction, "torrent");

namespace {

constexpr uint16_t dht_port = 43823;

// A UDP socket on the loopback interface standing in for a remote DHT node, so
// that no query leaves the host.
class scratch_node {
public:
  scratch_node() {
    m_fd = ::socket(AF_INET, SOCK_DGRAM, 0);
    CPPUNIT_ASSERT(m_fd >= 0);

    auto sa = torrent::sa_make_inet_h(0x7f000001, 0);

    CPPUNIT_ASSERT(::bind(m_fd, sa.get(), sizeof(sockaddr_in)) == 0);

    sockaddr_in bound      = {};
    socklen_t   sa_length  = sizeof(bound);

    CPPUNIT_ASSERT(::getsockname(m_fd, reinterpret_cast<sockaddr*>(&bound), &sa_length) == 0);

    m_port = ntohs(bound.sin_port);
  }

  ~scratch_node() {
    if (m_fd >= 0)
      ::close(m_fd);
  }

  scratch_node(const scratch_node&) = delete;
  scratch_node& operator=(const scratch_node&) = delete;

  int      fd() const   { return m_fd; }
  uint16_t port() const { return m_port; }

private:
  int      m_fd{-1};
  uint16_t m_port{0};
};

torrent::Object
create_dht_cache() {
  torrent::HashString self_id;
  self_id.clear(0x55);

  auto cache = torrent::Object::create_map();
  cache.insert_key("self_id", self_id.str());
  cache.insert_key("nodes", torrent::Object::create_map());

  return cache;
}

torrent::tracker::DhtController*
start_dht() {
  auto dht = torrent::runtime::network_manager()->dht_controller();

  dht->initialize(create_dht_cache());
  CPPUNIT_ASSERT(dht->start());

  return dht;
}

void
send_ping_query_to(const scratch_node& node) {
  auto sa = torrent::sa_make_inet_h(0x7f000001, 0);

  torrent::runtime::network_manager()->dht_add_peer_node(sa.get(), node.port());
}

std::string
receive_transaction_id(const scratch_node& node) {
  pollfd descriptor = {node.fd(), POLLIN, 0};

  CPPUNIT_ASSERT(::poll(&descriptor, 1, 1000) == 1);

  std::vector<char> buffer(1500);
  auto              size = ::recv(node.fd(), buffer.data(), buffer.size(), 0);

  CPPUNIT_ASSERT(size > 0);

  torrent::DhtMessage query;
  torrent::static_map_read_bencode(buffer.data(), buffer.data() + size, query);

  auto id = query[torrent::key_t].as_raw_string();

  return std::string(id.data(), id.size());
}

std::string
build_ping_reply(const std::string& transaction_id) {
  torrent::HashString node_id;
  node_id.clear(0x77);

  return "d1:rd2:id20:" + node_id.str() + "e1:t" + std::to_string(transaction_id.size()) + ":" +
    transaction_id + "1:y1:re";
}

void
send_reply(const scratch_node& node, uint16_t port, const std::string& reply) {
  auto sa   = torrent::sa_make_inet_h(0x7f000001, port);
  auto size = ::sendto(node.fd(), reply.data(), reply.size(), 0, sa.get(), sizeof(sockaddr_in));

  CPPUNIT_ASSERT(size == static_cast<ssize_t>(reply.size()));
}

} // namespace

void
test_dht_transaction::setUp() {
  TestFixtureWithMainNetTrackerThread::setUp();

  torrent::runtime::socket_manager()->set_max_size_and_adjust(1024);
  torrent::runtime::network_config()->set_override_dht_port(dht_port);
}

void
test_dht_transaction::tearDown() {
  torrent::runtime::network_manager()->dht_controller()->stop();

  TestFixtureWithMainNetTrackerThread::tearDown();
}

void
test_dht_transaction::test_key_includes_port() {
  auto first  = torrent::sa_make_inet_h(0x7f000001, 6881);
  auto second = torrent::sa_make_inet_h(0x7f000001, 6882);

  auto key = torrent::DhtTransaction::key(first.get(), 1);

  CPPUNIT_ASSERT(key != torrent::DhtTransaction::key(second.get(), 1));
  CPPUNIT_ASSERT(torrent::DhtTransaction::key_match(key, first.get()));
  CPPUNIT_ASSERT(!torrent::DhtTransaction::key_match(key, second.get()));
}

void
test_dht_transaction::test_query_transaction_id_is_two_bytes() {
  scratch_node node;

  start_dht();
  send_ping_query_to(node);
  m_main_thread->test_poll();

  CPPUNIT_ASSERT_EQUAL(size_t{2}, receive_transaction_id(node).size());
}

void
test_dht_transaction::test_reply_from_other_port_is_not_matched() {
  scratch_node node;
  scratch_node other_port;

  auto dht = start_dht();

  send_ping_query_to(node);
  m_main_thread->test_poll();

  auto reply = build_ping_reply(receive_transaction_id(node));

  send_reply(other_port, dht->port(), reply);
  m_main_thread->test_poll();

  CPPUNIT_ASSERT_EQUAL(0u, dht->get_statistics().replies_received);

  send_reply(node, dht->port(), reply);
  m_main_thread->test_poll();

  CPPUNIT_ASSERT_EQUAL(1u, dht->get_statistics().replies_received);
}
