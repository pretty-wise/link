/*
 * Copywrite 2014-2015 Krzysztof Stasik. All rights reserved.
 */
#include "gtest/gtest.h"

#include "base/network/socket.h"
#include "base/threading/thread.h"
#include "common/message_stream.h"
#include "link/link.h"

struct Data {
  Base::Socket::Handle out;
  Base::Socket::Handle in;
  Base::Socket::Handle listen;
};

class TestInitializer {
public:
  TestInitializer(Data &data) : m_data(data) { Initialize(data); }
  ~TestInitializer() { Terminate(m_data); }

private:
  Data &m_data;

  void Terminate(Data &data) {
    if(data.in != Base::Socket::InvalidHandle)
      Base::Socket::Close(data.in);
    if(data.out != Base::Socket::InvalidHandle)
      Base::Socket::Close(data.out);
    if(data.listen != Base::Socket::InvalidHandle)
      Base::Socket::Close(data.listen);

    data.in = Base::Socket::InvalidHandle;
    data.out = Base::Socket::InvalidHandle;
    data.listen = Base::Socket::InvalidHandle;
  }

  void Initialize(Data &data) {
    int error = 0;
    data.in = Base::Socket::InvalidHandle;
    data.out = Base::Socket::InvalidHandle;
    data.listen = Base::Socket::InvalidHandle;

    data.out = Base::Socket::Tcp::Open(&error);
    if(data.out == Base::Socket::InvalidHandle) {
      Base::Socket::Close(data.out);
      data.out = Base::Socket::InvalidHandle;
      FAIL() << "error opening socket: " << error;
      return;
    }

    data.listen = Base::Socket::Tcp::Open(&error);
    if(data.listen == Base::Socket::InvalidHandle) {
      Base::Socket::Close(data.out);
      data.out = Base::Socket::InvalidHandle;
      FAIL() << "error opening socket: " << error;
      return;
    }

    u16 port = 0;
    bool listens = Base::Socket::Tcp::Listen(data.listen, &port, &error);
    if(!listens || port == 0) {
      Base::Socket::Close(data.out);
      Base::Socket::Close(data.listen);
      data.out = Base::Socket::InvalidHandle;
      data.listen = Base::Socket::InvalidHandle;
      FAIL() << "error listening on socket: " << error;
      return;
    }

    Base::Url url("127.0.0.1", port);
    Base::Socket::Address address;
    bool address_created = Base::Socket::Address::CreateTCP(url, &address);
    if(!address_created) {
      Base::Socket::Close(data.out);
      Base::Socket::Close(data.listen);
      data.out = Base::Socket::InvalidHandle;
      data.listen = Base::Socket::InvalidHandle;
      FAIL();
      return;
    }

    int res = Base::Socket::Tcp::Connect(data.out, address);
    if(res == Base::Socket::Tcp::kFailed) {
      Base::Socket::Close(data.out);
      Base::Socket::Close(data.listen);
      data.out = Base::Socket::InvalidHandle;
      data.listen = Base::Socket::InvalidHandle;
      FAIL();
      return;
    }

    bool connected = false;

    Base::Socket::Address connectee;
    u32 timeout_ms = 1000;
    connected = Base::Socket::Tcp::Accept(data.listen, &data.in, &connectee,
                                          timeout_ms, &error);

    if(!connected) {
      Base::Socket::Close(data.out);
      Base::Socket::Close(data.listen);
      data.out = Base::Socket::InvalidHandle;
      data.listen = Base::Socket::InvalidHandle;
      FAIL() << "accept timed out: " << error;
    }
  }
};

const int header_size = 4;
const s8 message[] = "hello_world";
const streamsize message_size = sizeof(message);

TEST(ConnectionManager, WriteMessage) {
  Data data;
  TestInitializer init(data);
  ASSERT_TRUE(data.in != Base::Socket::InvalidHandle);
  ASSERT_TRUE(data.out != Base::Socket::InvalidHandle);
  ASSERT_TRUE(data.listen != Base::Socket::InvalidHandle);

  Link::MessageOutStream out(message_size);
  bool written = out.Write(message, message_size);
  ASSERT_TRUE(written == false);
}

TEST(ConnectionManager, ReadMessage) {
  Data data;
  TestInitializer init(data);
  ASSERT_TRUE(data.in != Base::Socket::InvalidHandle);
  ASSERT_TRUE(data.out != Base::Socket::InvalidHandle);
  ASSERT_TRUE(data.listen != Base::Socket::InvalidHandle);

  int message_count = 4;
  Link::MessageOutStream out((message_size + header_size) * message_count + 1);
  for(int i = 0; i < message_count; ++i) {
    bool written = out.Write(message, message_size);
    ASSERT_TRUE(written == true);
  }

  bool written = out.Write(message, 1);
  ASSERT_TRUE(written == false);

  bool res = out.Flush(data.out);
  ASSERT_TRUE(res);

  Link::MessageInStream in((message_size + header_size) * message_count);

  s8 dest[message_size];
  streamsize bread;
  streamsize total_read = 0;

  while(total_read < message_size * message_count) {
    res = in.Process(data.in);
    ASSERT_TRUE(res == Link::MessageInStream::RS_WOULDBLOCK);

    bread = 0;
    Result val = in.Read(dest, message_size, &bread);
    ASSERT_TRUE((val == RS_SUCCESS && bread == message_size) ||
                val == RS_EMPTY);
    total_read += bread;
  }

  ASSERT_TRUE(in.TotalMessageSize() == 0);
  ASSERT_TRUE(out.Empty());
}

TEST(ConnectionManager, ReadMessage2) {
  Data data;
  TestInitializer init(data);
  ASSERT_TRUE(data.in != Base::Socket::InvalidHandle);
  ASSERT_TRUE(data.out != Base::Socket::InvalidHandle);
  ASSERT_TRUE(data.listen != Base::Socket::InvalidHandle);

  streamsize msg_size = 5;
  u32 message_count = 40;
  s8 dest[msg_size];

  Link::MessageOutStream out((msg_size + header_size) * message_count + 1);
  Link::MessageInStream in((msg_size + header_size) * message_count);

  streamsize bytes_written = 0, bytes_read = 0, total = 2000;
  do {
    if(bytes_written < total) {
      bool res = out.Write(message, msg_size);
      bytes_written += msg_size;
      ASSERT_TRUE(res);
    }

    bool res = out.Flush(data.out);
    ASSERT_TRUE(res);

    res = in.Process(data.in);
    ASSERT_TRUE(res == Link::MessageInStream::RS_WOULDBLOCK);

    streamsize bread;
    Result ret = in.Read(dest, msg_size, &bread);
    ASSERT_TRUE((ret == RS_SUCCESS && bread == msg_size) || ret == RS_EMPTY);
    bytes_read += bread;

    /*
        if(ret == RS_SUCCESS) {
          for(u16 i = 0; i < msg_size; ++i) {
            ASSERT_TRUE(dest[i] == message[i]);
          }
        }
    */

    ASSERT_TRUE(in.NextMessageSize() <= in.TotalMessageSize());
    // printf("read/written - %d/%d. next: %d total: %d\n", bytes_read,
    //      bytes_written, in.NextMessageSize(), in.TotalMessageSize());
  } while(bytes_written < total || bytes_read < total);

  ASSERT_TRUE(in.NextMessageSize() == 0);
  ASSERT_TRUE(in.TotalMessageSize() == 0);
}
