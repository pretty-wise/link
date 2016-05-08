/*
 * Copywrite 2014-2015 Krzysztof Stasik. All rights reserved.
 */
#include "gtest/gtest.h"

#include "Link/Link.h"
#include "watch_manager.h"

//TODO: TEST(PluginWatch, PatterMatching) {}

TEST(PluginWatch, CreateFilterRemove) {

	const char* kPluginName = "plugin_A";
	const char* kPluginVersion = "1.0";

	Link::WatchManager watch;

	WatchHandle watch_A = watch.Create(kPluginName, kPluginVersion);
	ASSERT_TRUE(watch_A);

	std::vector<WatchHandle> results;

	watch.Filter(kPluginName, kPluginVersion, results);
	ASSERT_TRUE(results.size() == 1 && results[0] == watch_A);

	watch.Filter(kPluginName, "*", results);
	ASSERT_TRUE(results.size() == 1 && results[0] == watch_A);

	watch.Filter("*", kPluginVersion, results);
	ASSERT_TRUE(results.size() == 1 && results[0] == watch_A);

	watch.Filter("*", "version_mismatch", results);
	ASSERT_TRUE(results.empty());

	watch.Filter(kPluginName, "version_mismatch", results);
	ASSERT_TRUE(results.empty());

	watch.Filter("name_mismatch", kPluginVersion, results);
	ASSERT_TRUE(results.empty());

	watch.Remove(watch_A);
	watch.Filter(kPluginName, kPluginVersion, results);
	ASSERT_TRUE(results.empty());
}
