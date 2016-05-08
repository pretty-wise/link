/*
 * Copywrite 2014-2015 Krzysztof Stasik. All rights reserved.
 */
#pragma once

#include "base/core/types.h"
#include "base/network/socket.h"

namespace Link {

/* Processes data incoming on the socket, enables reading whole messages.
*/
class MessageInStream {
public:
  enum ProcessResult { RS_WOULDBLOCK, RS_ERROR, RS_EOF };

  MessageInStream(streamsize nbytes);
  ~MessageInStream();

  /// Reads a message from a stream.
  /// If RS_BUFFER_TOO_SMALL is retured read contains number of bytes needed.
  /// @param buffer Buffer to read data to.
  /// @param nbytes Size of the buffer.
  /// @param read Bytes read.
  /// @result RS_SUCCESS, RS_BUFFER_TOO_SMALL, RS_EMPTY
  Result Read(void *buffer, streamsize nbytes, streamsize *read);

  /// Reads data available on socket.
  /// @param socket Socket to read data from.
  ProcessResult Process(Base::Socket::Handle socket);

  // Returns current bytes of data in the queue.
  unsigned int TotalMessageSize() const;

  /// Returns the size of next message, in bytes.
  /// Beware that, in multi threaded application it's not guaranteed
  /// that the message won't be read by another thread.
  unsigned int NextMessageSize() const;

  bool Empty() const { return NextMessageSize() == 0; }

private:
  s8 *m_buffer_start;
  s8 *m_buffer_end;
  s8 *m_read;
  s8 *m_write;
  struct Header *m_cur_header;
  u32 m_num_messages;
  streamsize m_header_bytes_expected;
  streamsize m_message_bytes_expected;
  streamsize m_total_msg_size;

private:
  bool CanWrite(s8 *beg, s8 *end) const;
};

/* Writes messages for program, flushes them into socket. */
class MessageOutStream {
public:
  static const streamsize kMTU = 1024;
  MessageOutStream(streamsize nbytes);
  ~MessageOutStream();

  /// Writes a message to stream.
  /// @param buffer Message to write.
  /// @param nbytes Size of the message to write.
  /// @return True if write succeeded, false otherwise.
  bool Write(const void *buffer, streamsize nbytes);

  /// Checks if there is data to be flushed.
  /// @return True if there is data to be flushed.
  bool Empty() const;

  /// Flushes the remaining data to socket.
  /// @param socket Socket to write to.
  /// @return True if socket write succeeded, false otherwise. // todo: error?
  bool Flush(Base::Socket::Handle socket);

  void Clear();

private:
  s8 *m_buffer_start;
  s8 *m_buffer_end;
  s8 *m_read;
  s8 *m_write;

private:
  void WriteData(s8 *buffer, streamsize nbytes);
  streamsize GetBytesFree() const;
};

} // namespace Link
