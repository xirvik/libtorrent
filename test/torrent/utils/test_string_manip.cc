#include "config.h"

#include "test_string_manip.h"

#include <string>
#include <type_traits>

#include "torrent/utils/string_manip.h"

CPPUNIT_TEST_SUITE_NAMED_REGISTRATION(test_string_manip, "torrent/utils");

// The sentinel has to survive being returned and compared. Held in a char it
// does not: where char is unsigned it becomes 255, every '== -1' test is false
// and invalid hex is accepted.
void
test_string_manip::test_hex_error_sentinel() {
  static_assert(std::is_signed_v<decltype(torrent::utils::hex_to_value_or_error('0'))>,
                "the hex error sentinel must be returned in a signed type wider than char");
  static_assert(sizeof(torrent::utils::hex_to_value_or_error('0')) > sizeof(char),
                "the hex error sentinel must be returned in a signed type wider than char");

  CPPUNIT_ASSERT_EQUAL(-1, torrent::utils::hex_to_value_or_error('Z'));
  CPPUNIT_ASSERT_EQUAL(-1, torrent::utils::hex_to_value_or_error('\xff'));
  CPPUNIT_ASSERT_EQUAL(15, torrent::utils::hex_to_value_or_error('f'));
}

void
test_string_manip::test_from_hex_rejects_invalid() {
  const std::string source = "0123456789abcdef0123456789abcdef0123456Z";
  char              destination[20];

  auto end = torrent::utils::transform_from_hex(source.begin(), source.end(),
                                                destination, destination + sizeof(destination));

  CPPUNIT_ASSERT(end == destination);
}
