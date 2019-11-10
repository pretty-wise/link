/*
 * Copywrite 2014-2015 Krzysztof Stasik. All rights reserved.
 */

#include "base/core/macro.h"
#include "base/process/process.h"

#include <errno.h>

namespace Base {
namespace Process {

pid_t getpid() { return ::getpid(); }

pid_t fork() { return ::fork(); }

int exec(const char *path, char *const argv[]) { return ::execv(path, argv); }

pid_t spawn(const char *path, char *const argv[]) {
  pid_t child_pid = fork();
  if(child_pid < 0) {
    return -1; // error: fork() failed.
  }
  if(child_pid == 0) {
    int res = exec(path, argv);
    if(res == -1) {
      BASE_LOG("exec failed with errno: %d\n", errno);
      return -1;
    }
  }
  return child_pid;
}

pid_t spawn2(const char *path, char *const argv[], FileHandle &std_out) {
  const u32 pipe_read = 0;
  const u32 pipe_write = 1;

  FileHandle stdout_pipe[2];
  if(pipe(stdout_pipe) < 0) {
    return -1;
  }

  pid_t child_pid = fork();
  if(child_pid < 0) {
    return -1; // error
  }
  if(child_pid == 0) {
    // child

    // redirect stdout
    if(dup2(stdout_pipe[pipe_write], STDOUT_FILENO) == -1) {
      return -1;
    }
    // redirect stderr
    if(dup2(stdout_pipe[pipe_write], STDERR_FILENO) == -1) {
      return -1;
    }

    // close all that are for the parent only
    close(stdout_pipe[pipe_read]);
    close(stdout_pipe[pipe_write]);

    int res = exec(path, argv);
    if(res == -1) {
      BASE_LOG("exec failed with: %d\n", errno);
      return -1;
    }
  }
  // parent

  // close unused by parent
  close(stdout_pipe[pipe_write]);

  std_out = stdout_pipe[pipe_read];
  return child_pid;
}

} // namespace Process
} // namespace Base
