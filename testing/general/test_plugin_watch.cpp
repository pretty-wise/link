/*
 * Copywrite 2014-2015 Krzysztof Stasik. All rights reserved.
 */
#include "gtest/gtest.h"

#include "link/link.h"
#include "watch_manager.h"

// TODO: TEST(PluginWatch, PatterMatching) {}

TEST(PluginWatch, CreateFilterRemove) {

  const char *kPluginName = "plugin_A";
  const char *kPluginVersion = "1.0";
  const char *kHostname = "localhost";

  Link::WatchManager watch;

  std::vector<WatchHandle> results;

  {
    WatchHandle exactMatch =
        watch.Create(kPluginName, kPluginVersion, kHostname);
    ASSERT_TRUE(exactMatch);

    watch.Filter(kPluginName, kPluginVersion, kHostname, results);
    ASSERT_TRUE(results.size() == 1 && results[0] == exactMatch);

    watch.Remove(exactMatch);
  }

  {
    WatchHandle anyVersionMatch = watch.Create(kPluginName, "*", kHostname);
    ASSERT_TRUE(anyVersionMatch);

    watch.Filter(kPluginName, kPluginVersion, kHostname, results);
    ASSERT_TRUE(results.size() == 1 && results[0] == anyVersionMatch);

    watch.Remove(anyVersionMatch);
  }

  {
    WatchHandle anyNameMatch = watch.Create("*", kPluginVersion, kHostname);
    ASSERT_TRUE(anyNameMatch);

    watch.Filter(kPluginName, kPluginVersion, kHostname, results);
    ASSERT_TRUE(results.size() == 1 && results[0] == anyNameMatch);

    watch.Remove(anyNameMatch);
  }

  {
    WatchHandle versionMismatch =
        watch.Create("*", "versionmismatch", kHostname);
    ASSERT_TRUE(versionMismatch);

    watch.Filter(kPluginName, kPluginVersion, kHostname, results);
    ASSERT_TRUE(results.empty());

    watch.Remove(versionMismatch);
  }

  // empty check
  watch.Filter("*", "*", "*", results);
  ASSERT_TRUE(results.empty());
}
