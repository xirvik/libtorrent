#include "config.h"

#include "test/torrent/runtime/test_network_config.h"

#include "torrent/runtime/network_config.h"

CPPUNIT_TEST_SUITE_NAMED_REGISTRATION(test_network_config, "torrent/runtime");

void
test_network_config::test_block_private_peers_defaults_to_off() {
  CPPUNIT_ASSERT(!torrent::runtime::network_config()->is_block_private_peers());
}

void
test_network_config::test_block_private_peers_can_be_enabled() {
  auto config = torrent::runtime::network_config();

  CPPUNIT_ASSERT(!config->is_block_private_peers());

  config->set_block_private_peers(true);
  CPPUNIT_ASSERT(config->is_block_private_peers());

  config->set_block_private_peers(false);
  CPPUNIT_ASSERT(!config->is_block_private_peers());
}
