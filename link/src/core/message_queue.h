/*
 * Copywrite 2014-2015 Krzysztof Stasik. All rights reserved.
 */
#pragma once
#include "base/core/types.h"
#include "base/threading/mutex.h"
#include "base/memory/ring_allocator.h"

namespace Link {

/// Thread safe message queue.
class MessageQueue {
public:
	/// @param bytes Size of message queue buffer.
	MessageQueue(unsigned int bytes);
  
	~MessageQueue();

	/// Writes data to the queue.
	/// @param data Data to write.
	/// @param bytes Length of data, in bytes.
	/// @return true if write succeeded, false otherwise.
	bool Write(const void* data, unsigned int bytes);

	/// Reads data from message queue.
	/// If RS_BUFFER_TOO_SMALL is returned read contains number of bytes needed pending.
	/// @param data Buffer to read data to.
	/// @param max_length Maximum amount of data to read.
	/// @param read Number of bytes read.
	/// @result RS_SUCCESS, RS_BUFFER_TOO_SMALL, RS_EMPTY
	Result Read(void* data, unsigned int max_length, streamsize* read);

	// Returns current bytes of data in the queue.
	unsigned int TotalMessageSize() const { return m_total_message_bytes; }

	/// Returns the size of next message, in bytes.
	/// Beware that, in multi threaded application it's not guaranteed
	/// that the message won't be read by another thread.
	unsigned int NextMessageSize() const;

	bool Empty() const { return TotalMessageSize() == 0; }
private:
	typedef Base::Mutex ListLock;
	typedef Base::Mutex AllocLock;

	struct Message {
		Message* next;
		unsigned int size;
		char data[0]; // warning 4200.
	};

	void* m_queue_memory; //< Memory for ring allocator.
	Base::RingAllocator m_allocator; //< Used to allocate Message structures that hold data and links to next messages.
	mutable ListLock m_list_lock; //< Protects linked list operations.
	mutable AllocLock m_allocation_lock; //< Protects allocator operations.
	Message* m_message_list; //< Head of linked list of messages.
	unsigned int m_total_message_bytes; //< Total size of all messages waiting to be read.
};

} // namespace Link
