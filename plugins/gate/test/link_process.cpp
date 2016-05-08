/*
 * Copywrite 2014-2015 Krzysztof Stasik. All rights reserved.
 */
#include "link_process.h"
#include "base/process/process.h"
#include "base/threading/thread.h"
#include "base/core/log.h"
#include "base/core/assert.h"
#include "sys/wait.h"

#include <signal.h>
#include <string.h>

int read_line(int fd, char *buffer, int nbytes) {
  if(!buffer || nbytes <= 0) {
    return -1;
  }
  char c;
  int total_read = 0;
  while(1) {
    int nread = read(fd, &c, 1);
    if(nread == -1) {
      if(errno == EINTR) {
        continue;
      } else {
        return -1;
      }
    } else if(nread == 0) {
      // EOF reached
      if(total_read == 0) {
        return 0;
      } else {
        break;
      }
    } else {
      if(total_read < nbytes - 1) {
        ++total_read;
        *buffer++ = c;
      }
      if(c == '\n') {
        break;
      }
    }
  }

  *(buffer - 2) = '\0';
  return total_read;
}

Parser::Parser() {}

bool Parser::Run(const char *executable, char *const argv[]) {
  pipe(m_channel);
  m_pid = fork();
  if(m_pid == 0) {
    int result = dup2(m_channel[kOutputEnd], kStdOut);
    if(result < 0) {
      return false;
    }
    close(m_channel[kInputEnd]);
    close(m_channel[kOutputEnd]);
    Base::Process::exec(executable, argv);
  }
  close(m_channel[kOutputEnd]);
  fcntl(m_channel[kInputEnd], F_SETFL, O_NONBLOCK);
  return true;
}

void Parser::Kill() {
  int res = kill(m_pid, SIGTERM);
  BASE_ASSERT(res == 0);
  Read(10);
  wait(NULL);
  BASE_LOG("%s", m_output.c_str());
}

void Parser::Wait() {
  wait(NULL);
  Read(1000);
  BASE_LOG("%s", m_output.c_str());
}

void Parser::Read(int timeout_ms) {
  const int buffer_size = Base::Log::kLogLineMaxLength;
  char buffer[buffer_size];
  while(timeout_ms >= 0) {
    memset((void *)buffer, '\0', buffer_size);
    fflush(stdout);
    int nbytes = read_line(m_channel[kInputEnd], buffer, buffer_size);
    if(nbytes < 0 && (errno == 35 || errno == EAGAIN)) {
      int delta_ms = 100;
      timeout_ms -= delta_ms;
      usleep(delta_ms * 1000);
      continue;
    } else if(nbytes <= 0) {
      break;
    }
    m_output.append(buffer);
    m_output.append("\n");
  }
}

void Parser::ForEachLine(std::function<void(const std::string &)> func) {
  size_t beg = 0;
  do {
    // printf("%lu, %s\n", m_output.length(), m_output.c_str());
    size_t end = m_output.find_first_of('\n', beg);
    if(std::string::npos == end) {
      break;
    }
    std::string line = m_output.substr(beg, end - beg);
    beg = end + 1;
    func(line);
  } while(1);
}
