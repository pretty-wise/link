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

  WatchHandle watch_A = watch.Create(kPluginName, kPluginVersion, kHostname);
  ASSERT_TRUE(watch_A);

  std::vector<WatchHandle> results;

  watch.Filter(kPluginName, kPluginVersion, kHostname, results);
  ASSERT_TRUE(results.size() == 1 && results[0] == watch_A);

  watch.Filter(kPluginName, "*", kHostname, results);
  ASSERT_TRUE(results.size() == 1 && results[0] == watch_A);

  watch.Filter("*", kPluginVersion, kHostname, results);
  ASSERT_TRUE(results.size() == 1 && results[0] == watch_A);

  watch.Filter("*", "version_mismatch", kHostname, results);
  ASSERT_TRUE(results.empty());

  watch.Filter(kPluginName, "version_mismatch", kHostname, results);
  ASSERT_TRUE(results.empty());

  watch.Filter("name_mismatch", kPluginVersion, kHostname, results);
  ASSERT_TRUE(results.empty());

  watch.Remove(watch_A);
  watch.Filter(kPluginName, kPluginVersion, kHostname, results);
  ASSERT_TRUE(results.empty());
}
