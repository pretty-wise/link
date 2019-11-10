/*
 * Copywrite 2014-2015 Krzysztof Stasik. All rights reserved.
 */
#pragma once

#include <cstring> // size_t

namespace Base {

enum OpenMode {
  OM_Append = 0x1,
  OM_Read = 0x2,
  OM_Write = 0x4,
  OM_ReadWrite = 0x8,
  OM_Trunc = 0x10,
  OM_Create = 0x20
};

typedef int FileHandle;

static const FileHandle kInvalidHandle = -1;

// Create a file for writing. If the file exists
// its content is overwritten.
// @param path Path of file to create
// @return kInvalidHandle if operation failed.
FileHandle Create(const char *path);

// Create a file.
// @param path Path of file to open.
// @param mode OpenMode flags
// @return kInvalidHandle if operation failed.
FileHandle Open(const char *path, int mode);

// Close a file
// @param file Handle to close
void Close(FileHandle file);

// Read from handle.
// @param file Handle to read from.
// @param buffer Buffer to write to.
// @param nbytes Size of buffer in bytes.
// @return Number of bytes written.
size_t Read(FileHandle file, void *buffer, size_t nbytes);

// Write to file.
// @param buffer Buffer to write.
// @param nbytes Size of buffer in bytes
// @return Number of bytes written.
size_t Write(FileHandle file, const void *buffer, size_t nbytes);

// Get filesize.
// @param file Handle to get the size of.
// @return Size in bytes.
size_t Size(FileHandle file);

} // namespace Base
