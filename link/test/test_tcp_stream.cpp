/*
 * Copywrite 2014-2015 Krzysztof Stasik. All rights reserved.
 */
#include "gtest/gtest.h"

#include "link/link.h"
#include "base/network/socket.h"
#include "common/message_stream.h"
#include "base/threading/thread.h"

TEST(ConnectionManager, ConnectionStatus) {
	const int header_size = 4;
	//const streamsize stream_size = 1000;
	const s8 message[] = "hello_world";
	const streamsize message_size = sizeof(message);
 
	Base::Socket::Handle out_socket = Base::Socket::Tcp::Open();
	Base::Socket::Handle listen_socket = Base::Socket::Tcp::Open();
	Base::Socket::Handle in_socket = Base::Socket::Tcp::Open();
	ASSERT_TRUE(out_socket != 0 && listen_socket != 0 && in_socket != 0);

	u16 port = 0;
	bool listens = Base::Socket::Tcp::Listen(listen_socket, &port);
	ASSERT_TRUE(listens);
	ASSERT_TRUE(port != 0);
	
	int res = Base::Socket::Tcp::Connect(out_socket, Base::Url(Base::AddressIPv4(127, 0, 0, 1), port));
	ASSERT_TRUE(Base::Socket::Tcp::kFailed != res);

	Base::Thread::Sleep(10);

	bool connected = false;
	int i = 0;
	while(!connected) {
		Base::Url connectee;
		connected = Base::Socket::Tcp::Accept(listen_socket, &in_socket, &connectee);

		ASSERT_TRUE(i++ < 100);
	}
 
	{
		Link::MessageOutStream out(message_size); 
		bool written = out.Write(message, message_size);
		ASSERT_TRUE(written == false);
	}
 
	{
		int message_count = 4;
		Link::MessageOutStream out((message_size + header_size)*message_count + 1); 
		for(int i = 0; i < message_count; ++i){
			bool written = out.Write(message, message_size);
			ASSERT_TRUE(written == true);
		}

		bool written = out.Write(message, 1);
		ASSERT_TRUE(written == false);

		bool res = out.Flush(out_socket);
		ASSERT_TRUE(res);
	 
		Link::MessageInStream in((message_size + header_size)*message_count);

		s8 dest[message_size];
		streamsize bread;
		streamsize total_read = 0;

		while(total_read < message_size * message_count) {
			res = in.Process(in_socket);
			ASSERT_TRUE(res == Link::MessageInStream::RS_WOULDBLOCK);

			bread = 0;
			Result val = in.Read(dest, message_size, &bread);
			ASSERT_TRUE((val == RS_SUCCESS && bread == message_size) || val == RS_EMPTY);
			total_read += bread;
		}

		streamsize bytes_written = 0, bytes_read = 0, total = 2000, msg_size = 5;
		do {
			if(bytes_written < total) {
				bool res = out.Write(message, msg_size);
				bytes_written += msg_size;
				ASSERT_TRUE(res);
			}

			res = out.Flush(out_socket);
			ASSERT_TRUE(res);

			res = in.Process(in_socket);
			ASSERT_TRUE(res == Link::MessageInStream::RS_WOULDBLOCK);

			streamsize bread;
			Result ret = in.Read(dest, msg_size, &bread);
			ASSERT_TRUE((ret == RS_SUCCESS && bread == msg_size) || ret == RS_EMPTY);
			bytes_read += bread;

			ASSERT_TRUE(in.NextMessageSize() <= in.TotalMessageSize());
//			printf("read/written - %d/%d. next: %d total: %d\n", bytes_read, bytes_written, in.NextMessageSize(), in.TotalMessageSize());
		} while(bytes_written < total || bytes_read < total);

		ASSERT_TRUE(in.NextMessageSize() == 0);
		ASSERT_TRUE(in.TotalMessageSize() == 0);
	}
}
