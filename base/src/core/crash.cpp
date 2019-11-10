/*
 * Copywrite 2014-2015 Krzysztof Stasik. All rights reserved.
 */
#include "base/core/crash.h"
#include "base/memory/memory.h"
#include "base/process/process.h"
#include "base/threading/thread.h"
#include "base/core/macro.h"
#include "base/core/assert.h"
#include <signal.h>
#include <fcntl.h>
#include <execinfo.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>

#define mb(x) asm volatile("" : : : "memory")

namespace Base {

static bool terminate_flag = false;
static bool timeout_flag = false;

static const int kCrashMaxBacktraceDepth = 100;
static const int kMagicNumber = 0xdeadbeef;

struct crash_message {
  pid_t process_id;
  u32 signal;
  void *fault_address;
  void *backtrace[kCrashMaxBacktraceDepth];
  int backtrace_num;
  int magic_number;
  char additional_data[0];
};

static const int kCrashReadTimeout = 10;
static const int kCrashMaxMessageSize = 4096;

static int g_FaultWriteEnd = -1;

void fault_handler(int signal, siginfo_t *siginfo, void *context) {
  crash_message data;
  data.magic_number = kMagicNumber;

  data.signal = signal;

  if(siginfo) {
    data.fault_address = siginfo->si_addr;
  }

  data.backtrace_num = backtrace(data.backtrace, kCrashMaxBacktraceDepth);

  // pipe crash data to the daemon.
  int ret;
  do {
    ret = write(g_FaultWriteEnd, &data, sizeof(data));
  } while(ret && EINTR == errno);
  backtrace_symbols_fd(data.backtrace, data.backtrace_num, g_FaultWriteEnd);
  // backtrace_symbols_fd(data.backtrace, data.backtrace_num, STDOUT_FILENO);
  close(g_FaultWriteEnd);
  abort();
  return;
}

/* Set the FD_CLOEXEC flag to desc if value is nonzero,
 * or clear the flag if value is 0.
 * Return 0 on success, or -1 on error with errno set. */
int set_cloexec_flag(int desc, int value) {
  int oldflags = fcntl(desc, F_GETFD, 0);
  if(oldflags < 0) {
    return oldflags;
  }
  if(value != 0) {
    oldflags |= FD_CLOEXEC;
  } else {
    oldflags &= ~FD_CLOEXEC;
  }
  return fcntl(desc, F_SETFD, oldflags);
}

void handle_crash(crash_message *data) {
  BASE_LOG_LINE("Exception Caught at %p. Signal %d.", data->fault_address,
                data->signal);
  BASE_LOG_LINE("Stack Trace:");

  /*for(u32 i = 0; i < data->backtrace_num; ++i) {
    BASE_LOG_LINE("%016p", data->backtrace[i]);
  }*/

  BASE_LOG_LINE("%s", data->additional_data);
}

void crashd_main(int pipe_fd[]) {
  int ret = fork();
  int fd;
  if(ret) {
    return;
  } else {
    // clore write end.
    close(pipe_fd[1]);
    // store read end.
    fd = pipe_fd[0];
  }

  const int kChangeCWDToRoot = 0;
  const int kNoIOChange = 1;
  ret = daemon(kChangeCWDToRoot, kNoIOChange);
  if(-1 == ret) {
    return;
  }
  fd_set rfds;
  FD_ZERO(&rfds);
  FD_SET(fd, &rfds);

  // wait for data. ignore harmless signals.
  ret = select(fd + 1, &rfds, NULL, NULL, NULL);
  do {
    mb();
    if(terminate_flag)
      exit(0);
    ret = select(fd + 1, &rfds, NULL, NULL, NULL);
  } while(ret == -1 && EINTR == errno);

  if(-1 == ret) {
    return;
  }

  (void)alarm(kCrashReadTimeout);

  // data arrived. time to read it.
  int remaining_bytes = kCrashMaxMessageSize;
  static char data_received[kCrashMaxMessageSize];
  char *p = data_received;
  do {
    ret = read(fd, p, remaining_bytes);
    if(0 == ret || terminate_flag) {
      break;
    }
    if(timeout_flag) {
      break;
    }
    if(-1 == ret && EINTR == errno) {
      continue;
    }
    if(-1 == ret) {
      break;
    }
    p += ret;
    remaining_bytes -= ret;
  } while(1);

  close(fd);

  crash_message *data = reinterpret_cast<crash_message *>(data_received);
  if(p != data_received) {
    BASE_ASSERT(data->magic_number == kMagicNumber);
    handle_crash(data);
  }

  exit(0);
}

bool RegisterCrashHandler() {
  // create pipe and start a daemon process.
  int pipe_fd[2];
  if(g_FaultWriteEnd == -1) {
    int ret = pipe(pipe_fd);
    if(-1 == ret) {
      return false;
    }
    // store write end.
    g_FaultWriteEnd = pipe_fd[1];
    ret = set_cloexec_flag(g_FaultWriteEnd, 1);
    if(-1 == ret) {
      return false;
    }
    crashd_main(pipe_fd);
    // close read end.
    close(pipe_fd[0]);
  }

  struct sigaction act;
  memset(&act, 0, sizeof(act));
  act.sa_sigaction = fault_handler;
  sigfillset(&act.sa_mask);
  act.sa_flags = SA_SIGINFO;

  int ret = 0;
  ret = sigaction(SIGSEGV, &act, NULL);
  ret |= sigaction(SIGILL, &act, NULL);
  ret |= sigaction(SIGFPE, &act, NULL);
  ret |= sigaction(SIGBUS, &act, NULL);
  ret |= sigaction(SIGQUIT, &act, NULL);
  return ret == 0;
}

void Crash() { raise(SIGSEGV); }

} // namespace Base
