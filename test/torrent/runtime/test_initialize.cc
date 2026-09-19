#include "config.h"

#include "test_initialize.h"

#include <cstdio>
#include <sys/resource.h>
#include <sys/wait.h>
#include <unistd.h>

#include "torrent/exceptions.h"
#include "torrent/torrent.h"

CPPUNIT_TEST_SUITE_NAMED_REGISTRATION(test_initialize, "torrent/runtime");

namespace {

// A descriptor limit below the socket manager minimum makes initialize() throw after it has already
// allocated the manager, which is the state cleanup() has to recognise as "never initialized".
constexpr rlim_t too_few_descriptors = 256;

enum child_result {
  child_ok                         = 0,
  child_setrlimit_failed           = 20,
  child_main_thread_failed         = 21,
  child_initialize_succeeded       = 22,
  child_initialize_wrong_exception = 23,
  child_cleanup_returned           = 24,
  child_cleanup_wrong_exception    = 25,
};

[[noreturn]] void
run_child() {
  struct rlimit limit {
    too_few_descriptors, too_few_descriptors
  };

  if (::setrlimit(RLIMIT_NOFILE, &limit) != 0)
    ::_exit(child_setrlimit_failed);

  try {
    torrent::initialize_main_thread();
  } catch (...) {
    ::_exit(child_main_thread_failed);
  }

  try {
    torrent::initialize();
    ::_exit(child_initialize_succeeded);
  } catch (const torrent::base_error&) {
  } catch (...) {
    ::_exit(child_initialize_wrong_exception);
  }

  try {
    torrent::cleanup();
    ::_exit(child_cleanup_returned);
  } catch (const torrent::internal_error&) {
    ::_exit(child_ok);
  } catch (...) {
    ::_exit(child_cleanup_wrong_exception);
  }
}

} // namespace

// cleanup() decides whether the library is initialized by looking at the global manager pointer, so a
// failed initialize() must not leave that pointer set.
void
test_initialize::test_failed_initialize_leaves_library_uninitialized() {
  std::fflush(nullptr);

  pid_t pid = ::fork();
  CPPUNIT_ASSERT_MESSAGE("fork failed", pid != -1);

  if (pid == 0)
    run_child();

  int status{};
  CPPUNIT_ASSERT_EQUAL(pid, ::waitpid(pid, &status, 0));

  CPPUNIT_ASSERT_MESSAGE("cleanup() after a failed initialize() terminated on a signal", WIFEXITED(status));
  CPPUNIT_ASSERT_EQUAL(static_cast<int>(child_ok), WEXITSTATUS(status));
}
