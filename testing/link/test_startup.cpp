/*
 * Copywrite 2014-2015 Krzysztof Stasik. All rights reserved.
 */
#include "base/process/process.h"
#include "gtest/gtest.h"

#include <unistd.h>

std::string g_link_path;

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  if(argc != 2) {
    return -1;
  }
  g_link_path = argv[1];
  return RUN_ALL_TESTS();
}

TEST(Startup, SpawnChildAndReadStdOut) {
  char **args;
  Base::FileHandle stdout_handle;
  pid_t child_pid =
      Base::Process::spawn2(g_link_path.c_str(), args, stdout_handle);
  EXPECT_TRUE(child_pid > 0);

  u32 bytes_read = 0;
  const u32 buffer_size = 128;
  char buffer[buffer_size];

  const u32 bytes_to_read = 100;

  while(bytes_read < bytes_to_read) {
    memset(buffer, '\0', buffer_size);
    size_t read = Base::Read(stdout_handle, buffer, buffer_size);
    buffer[read] = '\0';
    bytes_read += read;
  }
  Base::Close(child_pid);
}
