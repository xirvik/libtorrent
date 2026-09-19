#include "config.h"

#include "test/net/test_udns_labels.h"

#include <cstdint>
#include <vector>

#include "net/udns_library.h"

CPPUNIT_TEST_SUITE_NAMED_REGISTRATION(test_udns_labels, "net");

namespace {

// Offset of the name being decoded, and the offset holding the literal name
// "test" that a compression pointer resolves to.
const unsigned name_offset = 12;
const unsigned target_offset = 20;

// A packet whose name at name_offset is a single byte of the given type
// followed by the offset of the literal name.
std::vector<uint8_t>
packet_with_label_type(uint8_t type) {
  std::vector<uint8_t> packet(64, 0);

  packet[name_offset] = type;
  packet[name_offset + 1] = target_offset;

  packet[target_offset] = 4;
  packet[target_offset + 1] = 't';
  packet[target_offset + 2] = 'e';
  packet[target_offset + 3] = 's';
  packet[target_offset + 4] = 't';
  packet[target_offset + 5] = 0;

  return packet;
}

int
decode(const std::vector<uint8_t>& packet) {
  auto    cur = packet.data() + name_offset;
  uint8_t name[DNS_MAXDN];

  return dns_getdn(packet.data(), &cur, packet.data() + packet.size(), name, sizeof(name));
}

} // namespace

// RFC 1035 gives the top two bits of a label length byte: 00 is a literal
// label and 11 a compression pointer, 01 and 10 are reserved. Decoding the
// reserved forms as pointers is what kept the label length check unreachable.
void
test_udns_labels::test_reserved_label_type() {
  CPPUNIT_ASSERT_EQUAL(-1, decode(packet_with_label_type(0x40)));
  CPPUNIT_ASSERT_EQUAL(-1, decode(packet_with_label_type(0x80)));
}

void
test_udns_labels::test_compression_pointer() {
  CPPUNIT_ASSERT_EQUAL(6, decode(packet_with_label_type(0xC0)));
}
