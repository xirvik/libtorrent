#include "config.h"

#include "test_download_manager.h"

#include <string>

#include "torrent/download/download_manager.h"
#include "torrent/hash_string.h"

CPPUNIT_TEST_SUITE_NAMED_REGISTRATION(test_download_manager, "torrent");

// find(const std::string&) reinterprets the string as a HashString and copies
// it into the comparison, so any string that is not exactly one info hash long
// is read past its end. The copy happens before the search does, so an empty
// manager is enough to reach it.
void
test_download_manager::test_find_rejects_wrong_length() {
  torrent::DownloadManager manager;

  for (size_t length = 0; length != torrent::HashString::size_data + 8; length++) {
    if (length == torrent::HashString::size_data)
      continue;

    CPPUNIT_ASSERT(manager.find(std::string(length, 'a')) == manager.end());
  }
}
