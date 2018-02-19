/*
 * Copywrite 2014-2015 Krzysztof Stasik. All rights reserved.
 */
#include "common/message_stream.h"

#include "base/core/assert.h"
#include "base/core/memory.h"
#include "base/network/socket.h"

namespace Link {

#if 0
#define STREAM_LOG BASE_LOG_LINE
#else
#define STREAM_LOG(...)
#endif

/// TCP message header.
struct Header {
  streamsize size; //! Size of the message.
};

/*

|
|H|CCCCCC|H|CCCCCCCCC|H|
|
^buffer_begin		^read_point
^process_point					^buffer_end
*/
namespace {

s8 *GetContent(Header *cur, s8 *buffer_beg, s8 *buffer_end) {
  s8 *content_beg = ((s8 *)cur) + sizeof(Header);
  s8 *content_end = content_beg + cur->size;
  if(content_end > buffer_end) {
    content_beg = buffer_beg;
  }
  return content_beg;
}

Header *GetNextHeader(Header *cur, s8 *buffer_beg, s8 *buffer_end) {
  BASE_ASSERT(cur->size > 0, "invalid size in header to get next header");

  Header *next_header =
      reinterpret_cast<Header *>(((s8 *)cur) + sizeof(Header) + cur->size);

  if((s8 *)next_header + sizeof(Header) > buffer_end) {
    next_header = reinterpret_cast<Header *>(buffer_beg);
  }
  return next_header;
}

streamsize BytesAvailable(s8 *start, s8 *end, s8 *read, s8 *write) {
  if(read == write) {
    return end - start - 1;
  }
  if(read < write) {
    return (end - write) + (read - start) - 1;
  }
  return read - write - 1;
}

} // anonymous namespace

MessageInStream::MessageInStream(streamsize nbytes)
    : m_buffer_start(nullptr), m_buffer_end(nullptr), m_read(nullptr),
      m_write(nullptr), m_cur_header(nullptr), m_num_messages(0),
      m_header_bytes_expected(sizeof(Header)), m_message_bytes_expected(0),
      m_total_msg_size(0) {
  BASE_ASSERT((u32)nbytes > sizeof(Header), "buffer too small");
  m_buffer_start = new s8[nbytes];
  m_buffer_end = m_buffer_start + nbytes;
  m_read = m_buffer_start;
  m_write = m_buffer_start;
}

MessageInStream::~MessageInStream() { delete[] m_buffer_start; }

Result MessageInStream::Read(void *buffer, streamsize nbytes,
                             streamsize *read) {
  *read = 0;

  if(m_num_messages == 0) {
    return RS_EMPTY;
  }

  Header *message = reinterpret_cast<Header *>(m_read);

  if(nbytes < message->size) {
    *read = message->size;
    return RS_BUFFER_TOO_SMALL;
  }

  // content pointer has to be valid, because m_num_messages > 0.
  BASE_ASSERT(message->size > 0);
  s8 *content = GetContent(message, m_buffer_start, m_buffer_end);

  memcpy(buffer, content, message->size);
  *read = message->size;

  m_read = reinterpret_cast<s8 *>(
      GetNextHeader(message, m_buffer_start, m_buffer_end));

  --m_num_messages;
  m_total_msg_size -= message->size;
  return RS_SUCCESS;
}

MessageInStream::ProcessResult
MessageInStream::Process(Base::Socket::Handle socket) {
  int error = 0;

  if(m_header_bytes_expected == 0 && m_message_bytes_expected == 0) {
    return RS_ERROR; // error occured in previous processing.
  }

  do {
    // read header
    streamsize avail =
        BytesAvailable(m_buffer_start, m_buffer_end, m_read, m_write);
    if(avail < m_header_bytes_expected) {
      return RS_WOULDBLOCK; // not enough memory, need to read first
    }

    u32 check_size = 0;
    if(m_num_messages > 0) {
      Header *message = reinterpret_cast<Header *>(m_read);
      check_size = message->size;
    }

    while(m_header_bytes_expected > 0) {
      streamsize bytes_read = Base::Socket::Tcp::Recv(
          socket, m_write, m_header_bytes_expected, &error);

      STREAM_LOG(
          "recv on socket %d read %dbytes (header) bytes left: %d. error: %d.",
          socket, bytes_read, m_header_bytes_expected - bytes_read,
          bytes_read == -1 ? errno : 0);

      if(bytes_read == -1 && error == EWOULDBLOCK) {
        return RS_WOULDBLOCK; // nothing to read
      } else if(bytes_read == 0) {
        return RS_EOF; // socket closed
      } else if(bytes_read == -1) {
        STREAM_LOG("recv error on socket %d: %d.", socket, error);
        // invalidating internal state
        m_header_bytes_expected = 0;
        m_message_bytes_expected = 0;
        return RS_ERROR;
      }

      if(check_size > 0) {
        Header *message = reinterpret_cast<Header *>(m_read);
        BASE_ASSERT(check_size == message->size);
      }

      BASE_ASSERT(m_write + bytes_read <= m_buffer_end, "buffer overflow");
      m_write += bytes_read;
      m_header_bytes_expected -= bytes_read;

      if(m_header_bytes_expected == 0) {
        // just read header. fixup data pointer.
        Header *message = reinterpret_cast<Header *>(m_write - sizeof(Header));
        message->size = Base::NetToHostL(message->size);
        BASE_ASSERT(message->size > 0, "message size must be positive");

        m_cur_header = message;
        m_message_bytes_expected = message->size;

        m_write = GetContent(message, m_buffer_start, m_buffer_end);
        if(!CanWrite(m_write,
                     m_write + message->size)) { // todo: check if write is not
                                                 // writing after m_read.
          m_header_bytes_expected = 0;
          m_message_bytes_expected = 0;
          return RS_ERROR; // error - no more space.
        }
      }
    }

    // read message
    while(m_message_bytes_expected > 0) {
      streamsize avail =
          BytesAvailable(m_buffer_start, m_buffer_end, m_read, m_write);
      Header *next_header =
          GetNextHeader(m_cur_header, m_buffer_start, m_buffer_end);
      if(avail < m_message_bytes_expected ||
         reinterpret_cast<s8 *>(next_header) == m_read) {
        // todo(kstasik): check if we are not trying to read a message that will
        // never fit the buffer
        return RS_WOULDBLOCK; // not enough memory, need to read first
      }

      streamsize bytes_read = Base::Socket::Tcp::Recv(
          socket, m_write, m_message_bytes_expected, &error);
      STREAM_LOG("recv on socket %d read %dbytes (message) bytes left: %d.",
                 socket, bytes_read, m_message_bytes_expected - bytes_read);
      if(bytes_read == -1 && error == EWOULDBLOCK) {
        return RS_WOULDBLOCK; // nothing to read.
      } else if(bytes_read == 0) {
        return RS_EOF;
      } else if(bytes_read == -1) {
        STREAM_LOG("recv error on socket %d: %d.", socket, error);
        m_header_bytes_expected = 0;
        m_message_bytes_expected = 0;
        return RS_ERROR;
      }

      if(check_size > 0) {
        Header *message = reinterpret_cast<Header *>(m_read);
        BASE_ASSERT(check_size == message->size);
      }

      BASE_ASSERT(m_write + bytes_read <= m_buffer_end, "buffer overflow");
      m_write += bytes_read;

      m_message_bytes_expected -= bytes_read;
      if(m_message_bytes_expected == 0) {
        // just read data. fixup header pointer.
        m_total_msg_size += m_cur_header->size;
        BASE_ASSERT(m_cur_header->size > 0, "message cannot be empty");

        m_cur_header =
            GetNextHeader(m_cur_header, m_buffer_start, m_buffer_end);
        BASE_ASSERT(m_read != reinterpret_cast<s8 *>(m_cur_header),
                    "buffer closed");
        m_write = reinterpret_cast<s8 *>(m_cur_header);
        // todo: check if m_cur_header does not write after m_read.
        if(!CanWrite(m_write, m_write + sizeof(Header))) {
          m_header_bytes_expected = 0;
          m_message_bytes_expected = 0;
          return RS_ERROR; // error - no more space for header.
        }

        m_header_bytes_expected = sizeof(Header);
        // expose message for reading
        ++m_num_messages;
      }
    }
  } while(1);

  // not reached.
  return RS_ERROR;
} // namespace Link

unsigned int MessageInStream::TotalMessageSize() const {
  return m_total_msg_size;
}

unsigned int MessageInStream::NextMessageSize() const {
  if(m_num_messages == 0) {
    return 0;
  }

  Header *message = reinterpret_cast<Header *>(m_read);
  return message->size;
}

bool MessageInStream::CanWrite(s8 *beg, s8 *end) const {
  /*BASE_ASSERT(beg >= m_buffer_start, "invald buffer start %p/%p", beg,
              m_buffer_start);
  BASE_ASSERT(end < m_buffer_end, "invalid buffer end %p/%p", end,
              m_buffer_end);*/

  if(beg < m_buffer_start || end > m_buffer_end) {
    return false; // stream might have received corrupted data.
  }

  // beg >= m_write && end >= m_write
  // OR
  // beg < m_write && end < m_read
  if(end > beg && (beg >= m_write || end < m_read)) {
    return true;
  }
  return false;
}

//---------------------------------------------------------------------------------

MessageOutStream::MessageOutStream(streamsize nbytes)
    : m_buffer_start(nullptr), m_buffer_end(nullptr) {
  m_buffer_start = new s8[nbytes];
  m_buffer_end = m_buffer_start + nbytes;
  // init empty header.
  m_write = m_buffer_start;
  m_read = m_buffer_start;
}

MessageOutStream::~MessageOutStream() { delete[] m_buffer_start; }

streamsize MessageOutStream::GetBytesFree() const {
  if(m_read > m_write) {
    return m_read - m_write;
  } else {
    return m_buffer_end - m_write + m_read - m_buffer_start;
  }
}

void MessageOutStream::WriteData(s8 *buffer, streamsize nbytes) {
  if((m_write >= m_read) && (m_buffer_end - m_write < nbytes)) {
    // two part write
    if(m_buffer_end - m_write > 0) { // m_write might be at m_buffer_end
      memcpy(m_write, buffer, m_buffer_end - m_write);
    }
    streamsize bytes_left = nbytes - (m_buffer_end - m_write);
    memcpy(m_buffer_start, buffer + (m_buffer_end - m_write), bytes_left);
    m_write = m_buffer_start + bytes_left;
  } else {
    // just append
    memcpy(m_write, buffer, nbytes);
    m_write += nbytes;
  }
}

bool MessageOutStream::Write(const void *buffer, streamsize nbytes) {
  streamsize bytes_required = nbytes + sizeof(Header);
  streamsize bytes_free = GetBytesFree();

  if(bytes_free <
     bytes_required + 1) { // +1 always protects from closing the buffer
    return false;
  }

  Header header;
  header.size = Base::HostToNetL(nbytes);
  WriteData((s8 *)&header, sizeof(Header));
  WriteData((s8 *)buffer, nbytes);
  BASE_ASSERT(m_write != m_read, "write point must not reach read point %p %p",
              m_write, m_read);

  return true;
}

bool MessageOutStream::Empty() const { return m_read == m_write; }

void MessageOutStream::Clear() {
  m_write = m_buffer_start;
  m_read = m_buffer_start;
}

bool MessageOutStream::Flush(Base::Socket::Handle socket) {

  do {
    streamsize to_send = 0;

    if(m_write >= m_read) {
      to_send = m_write - m_read;
    } else {
      to_send = m_buffer_end - m_read;
      if(to_send == 0) {
        m_read = m_buffer_start;
        to_send = m_write - m_read;
      }
    }

    if(to_send > kMTU) {
      to_send = kMTU; // clamp to MTU.
    }

    if(to_send == 0) {
      return true; // nothing to send.
    }
    int error = 0;
    streamsize sent = Base::Socket::Tcp::Send(socket, m_read, to_send, &error);
    if(sent == -1 && error != EWOULDBLOCK) {
      // todo(kstaik): re-enable LINK_ERROR("socket send failed with %d - %s",
      // error, Base::Socket::ErrorToString(error));
      return false;
    }

    m_read += sent;
  } while(1);
}

} // namespace Link
