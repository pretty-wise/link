/*
 * Copywrite 2014-2015 Krzysztof Stasik. All rights reserved.
 */
#include "base/io/base_file.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include "base/core/macro.h"

namespace Base {

int EngineToLinux(int mode) {
  int result = 0;
  struct {
    int engine;
    int linux;
  } mapping[] = {{OM_Append, O_APPEND}, {OM_Read, O_RDONLY},
                 {OM_Write, O_WRONLY},  {OM_ReadWrite, O_RDWR},
                 {OM_Trunc, O_TRUNC},   {OM_Create, O_CREAT}};
  for(u32 i = 0; i < sizeof(mapping) / sizeof(mapping[0]); ++i) {
    if((mode & mapping[i].engine) != 0) {
      result |= mapping[i].linux;
    }
  }
  return result;
}

FileHandle Create(const char *path) {
  return Open(path, OM_Write | OM_Create | OM_Trunc);
}

FileHandle Open(const char *path, int mode) {
  mode_t permissions = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;
  int handle = open(path, EngineToLinux(mode), permissions);
  return handle;
}

void Close(FileHandle file) {
  int result = close(file);
  if(result == -1) {
    BASE_LOG_LINE("file close failed with %d", errno);
  }
}

size_t Read(FileHandle file, void *buffer, size_t nbytes) {
  size_t nread = read(file, buffer, nbytes);
  if(nread == static_cast<size_t>(-1)) {
    BASE_LOG_LINE("file read failed with %d", errno);
  }
  return nread;
}

size_t Write(FileHandle file, const void *buffer, size_t nbytes) {
  size_t nwritten = write(file, buffer, nbytes);
  if(nwritten == static_cast<size_t>(-1)) {
    BASE_LOG_LINE("file write failed with %d", errno);
  }
  return nwritten;
}

size_t Size(FileHandle file) {
  struct stat file_stat;
  file_stat.st_size = 0;
  int result = fstat(file, &file_stat);
  if(result == -1) {
    BASE_LOG_LINE("fstat failed with %d", errno);
  }
  return file_stat.st_size;
}

} // namespace Base
