/*
 * Copywrite 2014-2015 Krzysztof Stasik. All rights reserved.
 */
#include "gtest/gtest.h"
#include "plugin/gate/gate_client.h"
#include "base/core/macro.h"
#include "base/threading/thread.h"
#include "base/core/log.h"
#include "link_process.h"

#include <vector>
#include <algorithm>

static const Base::LogChannel kTestLog("gate_test");

struct TestInstance {
	bool connected;
	std::vector<struct Link::Gate::user_t*> users;

	bool IsLoggedIn(struct Link::Gate::user_t* user) { return std::find(users.begin(), users.end(), user) != users.end(); }
};

void on_connected(struct Link::Gate::context_t* ctx, void* udata) {
	TestInstance* inst = static_cast<TestInstance*>(udata);
	ASSERT_TRUE(inst->connected == false);
	inst->connected = true;
	BASE_INFO(kTestLog, "connected");
}

void on_disconnected(struct Link::Gate::context_t* ctx, int reason, void* udata) {
	TestInstance* inst = static_cast<TestInstance*>(udata);
	ASSERT_TRUE(inst->connected == true);
	inst->connected = false;
	BASE_INFO(kTestLog, "disconnected. reason: %s", Link::Gate::Reason::ToString(reason));
}

void on_login(struct Link::Gate::context_t* ctx, struct Link::Gate::user_t* user, void* udata) {
	TestInstance* inst = static_cast<TestInstance*>(udata);
	BASE_INFO(kTestLog, "logged in %p", user);
	ASSERT_FALSE(inst->IsLoggedIn(user));
	inst->users.push_back(user);
}

void on_logout(struct Link::Gate::context_t* ctx, struct Link::Gate::user_t* user, void* udata) {
	TestInstance* inst = static_cast<TestInstance*>(udata);
	BASE_INFO(kTestLog, "logged out %p", user);
	ASSERT_TRUE(inst->IsLoggedIn(user));
	inst->users.erase(std::find(inst->users.begin(), inst->users.end(), user));
}

char* g_executable = nullptr;
char* g_config = nullptr;

Link::Gate::callbacks_t g_cbs = {
	on_connected,
	on_disconnected,
	on_login,
	on_logout
};

Link::Gate::config_t g_conf = {
	Link::Gate::kDefaultConnectTimeoutMs
};

const char* g_username = "prettywise";

int main(int argc, char* argv[]) {
	BASE_INFO(kTestLog, "info 123");
	// disable stdout buffering
	setbuf(stdout, nullptr);
	g_executable = argv[1]; 
	g_config = argv[2];
	BASE_INFO(kTestLog, "running gate client... %s (%s)", g_executable, g_config);
	if(!g_executable || !g_config) {
		return -1;
	}

	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}

bool read_gate_port(const std::string& line, int& portno) {
	const char* search_phrase = "gate listening on port: ";
	size_t pos = line.find(search_phrase);
	if(std::string::npos != pos) {
		size_t beg = pos + strlen(search_phrase);
		size_t size = line.find(" ", beg)-beg;
		std::string port = line.substr(beg, size);
		if(!port.empty()) {
			portno = atoi(port.c_str());
			return true;
		}
	}
	return false;
}

TEST(Gate, Init) {
	TestInstance instance;

	struct Link::Gate::context_t* gate = Link::Gate::Create(g_cbs, g_conf, (void*)&instance);
	ASSERT_TRUE(gate != nullptr);

	char* const argv[] = {g_executable, g_config, nullptr };
	Parser parser;
	ASSERT_TRUE(parser.Run(g_executable, argv));
	parser.Read(100);
	
	Link::Gate::Destroy(gate);
	parser.Kill();
}

TEST(Gate, Connect) {
	TestInstance instance;

	struct Link::Gate::context_t* gate = Link::Gate::Create(g_cbs, g_conf, (void*)&instance);
	ASSERT_TRUE(gate != nullptr);

	char* const argv[] = {g_executable, g_config, nullptr };

	Parser parser;
	ASSERT_TRUE(parser.Run(g_executable, argv));
	parser.Read(100);
	int portno = 0;
	bool found = false;
	parser.ForEachLine([&](const std::string& line) {
		if(!found) {
			found = read_gate_port(line, portno);
		}
	});
	
	BASE_INFO(kTestLog, "portno: %d", portno);
	ASSERT_NE(portno, 0);

	Base::Url url("127.0.0.1", portno);
	ASSERT_TRUE(Link::Gate::Connect(gate, url));
	
	Base::Thread::Sleep(100);
	Link::Gate::Update(gate);

	ASSERT_TRUE(Link::Gate::IsConnected(gate));

	Link::Gate::Disconnect(gate);

	ASSERT_FALSE(Link::Gate::IsConnected(gate));

	Link::Gate::Destroy(gate);
	parser.Kill();
}
TEST(Gate, LoginAndShutdown) {
	TestInstance instance;

	struct Link::Gate::context_t* gate = Link::Gate::Create(g_cbs, g_conf, (void*)&instance);
	ASSERT_TRUE(gate != nullptr);

	char* const argv[] = {g_executable, g_config, nullptr };

	Parser parser;
	ASSERT_TRUE(parser.Run(g_executable, argv));
	parser.Read(100);
	int portno = 0;
	bool found = false;
	parser.ForEachLine([&](const std::string& line) {
		if(!found) {
			found = read_gate_port(line, portno);
		}
	});
	
	BASE_INFO(kTestLog, "portno: %d", portno);
	ASSERT_NE(portno, 0);

	Base::Url url("127.0.0.1", portno);
	ASSERT_TRUE(Link::Gate::Connect(gate, url));
	
	Base::Thread::Sleep(100);
	Link::Gate::Update(gate);

	ASSERT_TRUE(Link::Gate::IsConnected(gate));
	
	struct Link::Gate::user_t* user = Link::Gate::Login(gate, g_username);
	ASSERT_TRUE(user != nullptr);

	Base::Thread::Sleep(100);
	Link::Gate::Update(gate);
	Base::Thread::Sleep(100);
	Link::Gate::Update(gate);
	ASSERT_TRUE(instance.IsLoggedIn(user));

	Link::Gate::Destroy(gate);
	parser.Kill();
	
	ASSERT_FALSE(instance.IsLoggedIn(user));
}

TEST(Gate, LoginMultiple) {
	TestInstance instance;

	struct Link::Gate::context_t* gate = Link::Gate::Create(g_cbs, g_conf, (void*)&instance);
	ASSERT_TRUE(gate != nullptr);

	char* const argv[] = {g_executable, g_config, nullptr };

	Parser parser;
	ASSERT_TRUE(parser.Run(g_executable, argv));
	parser.Read(100);
	int portno = 0;
	bool found = false;
	parser.ForEachLine([&](const std::string& line) {
		if(!found) {
			found = read_gate_port(line, portno);
		}
	});
	
	BASE_INFO(kTestLog, "portno: %d", portno);
	ASSERT_NE(portno, 0);

	Base::Url url("127.0.0.1", portno);
	ASSERT_TRUE(Link::Gate::Connect(gate, url));
	
	Base::Thread::Sleep(100);
	Link::Gate::Update(gate);

	ASSERT_TRUE(Link::Gate::IsConnected(gate));
	
	struct Link::Gate::user_t* user = Link::Gate::Login(gate, g_username);
	ASSERT_TRUE(user != nullptr);

	Base::Thread::Sleep(50);
	Link::Gate::Update(gate);
	Base::Thread::Sleep(50);
	Link::Gate::Update(gate);
	ASSERT_TRUE(instance.IsLoggedIn(user));

	ASSERT_TRUE(Link::Gate::Logout(gate, user));
	Base::Thread::Sleep(50);
	Link::Gate::Update(gate);
	Base::Thread::Sleep(50);
	Link::Gate::Update(gate);

	ASSERT_FALSE(instance.IsLoggedIn(user));
	ASSERT_EQ(instance.users.size(), 0);

	Link::Gate::Destroy(gate);
	parser.Kill();
	ASSERT_FALSE(instance.IsLoggedIn(user));	
}
TEST(Gate, MultiConnect) {
	TestInstance instance;

	struct Link::Gate::context_t* gate = Link::Gate::Create(g_cbs, g_conf, (void*)&instance);
	ASSERT_TRUE(gate != nullptr);

	char* const argv[] = {g_executable, g_config, nullptr };

	Parser parser;
	ASSERT_TRUE(parser.Run(g_executable, argv));
	parser.Read(100);
	int portno = 0;
	bool found = false;
	parser.ForEachLine([&](const std::string& line) {
		if(!found) {
			found = read_gate_port(line, portno);
		}
	});
	
	BASE_INFO(kTestLog, "portno: %d", portno); 
	Base::Url url("127.0.0.1", portno);

	{
		SCOPED_TRACE("first connect");
		ASSERT_TRUE(Link::Gate::Connect(gate, url));
		Base::Thread::Sleep(100);
		Link::Gate::Update(gate);
	}
	{
		SCOPED_TRACE("consecutive connect");
		// consecutive connect has to fail.
		ASSERT_FALSE(Link::Gate::Connect(gate, url));
		Link::Gate::Update(gate);
	}
	{
		SCOPED_TRACE("disconnect");
		Link::Gate::Disconnect(gate);
		Base::Thread::Sleep(100);
		ASSERT_TRUE(Link::Gate::Connect(gate, url));
		Link::Gate::Update(gate);
	}
	Link::Gate::Destroy(gate);
	parser.Kill();	
}

TEST(Gate, Message) {
	TestInstance instance;

	struct Link::Gate::context_t* gate = Link::Gate::Create(g_cbs, g_conf, (void*)&instance);
	ASSERT_TRUE(gate != nullptr);

	char* const argv[] = {g_executable, g_config, nullptr };

	Parser parser;
	ASSERT_TRUE(parser.Run(g_executable, argv));
	parser.Read(1000);
	int portno = 0;
	bool found = false;
	parser.ForEachLine([&](const std::string& line) {
		if(!found) {
			found = read_gate_port(line, portno);
		}
	});
	
	BASE_INFO(kTestLog, "portno: %d", portno); 
	Base::Url url("127.0.0.1", portno);

	ASSERT_TRUE(Link::Gate::Connect(gate, url));
	Base::Thread::Sleep(100);

	Link::Gate::Update(gate);

	Link::Gate::Destroy(gate);	
	parser.Kill();
}
