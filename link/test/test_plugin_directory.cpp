/*
 * Copywrite 2014-2015 Krzysztof Stasik. All rights reserved.
 */
#include "gtest/gtest.h"

#include "link/link.h"
#include "plugin_directory.h"
#include "base/core/macro.h"

TEST(PluginDirectory, Registration) {
	Link::PluginDirectory dir;
	PluginInfo info_A;
	Link::PluginDirectory::Initialize(info_A, "info_A", "1.0", "localhost", 0, 0);

//	BASE_LOG_LINE("name %s version %s", info_A.name, info_A.version);

	PluginHandle handle_A = dir.GenerateHandle(info_A);
	ASSERT_TRUE(handle_A);
	ASSERT_TRUE(handle_A == dir.GenerateHandle(info_A)); // same handle generated twice.

	bool result = dir.Register(handle_A, info_A);
	ASSERT_TRUE(result);
	ASSERT_TRUE(!dir.Register(handle_A, info_A)); // double registration.

	dir.Unregister(handle_A);

	bool reregistered = dir.Register(handle_A, info_A); // re-registration
	ASSERT_TRUE(reregistered);
	ASSERT_TRUE(!dir.Register(handle_A, info_A)); // double re-registration.


	PluginInfo info_B;
	Link::PluginDirectory::Initialize(info_B, "info_B", "1.0", "localhost", 0, 0);

	PluginHandle handle_B = dir.GenerateHandle(info_B);
	ASSERT_TRUE(handle_B != handle_A);
	ASSERT_TRUE(handle_B == dir.GenerateHandle(info_B)); // same handle generated twice.

	ASSERT_TRUE(!dir.Register(handle_A, info_B)); // register old handle with new info.

	result = dir.Register(handle_B, info_B);
	ASSERT_TRUE(result);
	ASSERT_TRUE(!dir.Register(handle_B, info_B)); // double re-registration.
	ASSERT_TRUE(!dir.Register(handle_B, info_A)); // double re-registration, diff info.
}

class AvailabilityListener : public Link::PluginDirectoryListener {
public:
	AvailabilityListener(PluginHandle plugin, PluginInfo info)
	: m_plugin(plugin)
	, m_available(false)
	, m_error(false) {
		Link::PluginDirectory::Copy(m_info, info);
	}
	void OnPluginAvailable(PluginHandle plugin, const PluginInfo& info){
		if(m_error) return;
		if(m_plugin == plugin){

			if(!Link::PluginDirectory::Equal(m_info, info)) {
				m_error = true;
				return;
			}

			if(m_available == true) {
				m_error = true;
				return;
			}
			m_available = true;
		}
	}
	void OnPluginUnavailable(PluginHandle plugin, const PluginInfo& info) {
		if(m_error) return;
		if(m_plugin == plugin) {
			if(m_available == false) {
				m_error = true;
				return;
			}
			m_available = false;
		}
	}

	bool HasError() const { return m_error; }
	bool IsAvailable() const { return m_available; }
private:
	PluginHandle m_plugin;
	PluginInfo m_info;
	bool m_available;
	bool m_error;
};

TEST(PluginDirectory, Listeners) {
	Link::PluginDirectory dir;

	PluginInfo info_A, info_B;
	Link::PluginDirectory::Initialize(info_A, "info_A", "1.0", "localhost", 0, 0);
	Link::PluginDirectory::Initialize(info_B, "info_B", "1.0", "localhost", 0, 0);
	PluginHandle handle_A = dir.GenerateHandle(info_A);
	PluginHandle handle_B = dir.GenerateHandle(info_B);

	AvailabilityListener check_A(handle_A, info_A);
	AvailabilityListener check_B(handle_B, info_B);

	dir.AddListener(&check_A);
	dir.AddListener(&check_B);

	ASSERT_TRUE(!check_A.IsAvailable() && !check_A.HasError());
	ASSERT_TRUE(!check_B.IsAvailable() && !check_B.HasError());

	bool registered = dir.Register(handle_A, info_A);
	ASSERT_TRUE(registered);
	ASSERT_TRUE(check_A.IsAvailable() && !check_A.HasError());
	ASSERT_TRUE(!check_B.IsAvailable() && !check_B.HasError());

	registered = dir.Register(handle_B, info_B);
	ASSERT_TRUE(registered);
	ASSERT_TRUE(check_A.IsAvailable() && !check_A.HasError());
	ASSERT_TRUE(check_B.IsAvailable() && !check_B.HasError());

	dir.Unregister(handle_A);
	ASSERT_TRUE(!check_A.IsAvailable() && !check_A.HasError());
	ASSERT_TRUE(check_B.IsAvailable() && !check_B.HasError());

	dir.Unregister(handle_A);
	ASSERT_TRUE(!check_A.IsAvailable() && !check_A.HasError());
	ASSERT_TRUE(check_B.IsAvailable() && !check_B.HasError());

	registered = dir.Register(handle_B, info_B);
	ASSERT_TRUE(!registered);
	ASSERT_TRUE(!check_A.IsAvailable() && !check_A.HasError());
	ASSERT_TRUE(check_B.IsAvailable() && !check_B.HasError());

	registered = dir.Register(handle_A, info_A);
	ASSERT_TRUE(registered);
	ASSERT_TRUE(check_A.IsAvailable() && !check_A.HasError());
	ASSERT_TRUE(check_B.IsAvailable() && !check_B.HasError());


	dir.Unregister(handle_A);
	ASSERT_TRUE(!check_A.IsAvailable() && !check_A.HasError());
	ASSERT_TRUE(check_B.IsAvailable() && !check_B.HasError());

	dir.Unregister(handle_B);
	ASSERT_TRUE(!check_A.IsAvailable() && !check_A.HasError());
	ASSERT_TRUE(!check_B.IsAvailable() && !check_B.HasError());

	dir.RemoveListener(&check_A);
	dir.RemoveListener(&check_B);
}

