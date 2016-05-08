/*
 * Copywrite 2014-2015 Krzysztof Stasik. All rights reserved.
 */
#include "message_queue.h"

#include <stdlib.h>
#include <string.h>

namespace Link {

MessageQueue::MessageQueue(unsigned int bytes)
	: m_queue_memory(malloc(bytes))
	, m_allocator(m_queue_memory, bytes)
	, m_message_list(nullptr)
	, m_total_message_bytes(0)
{
	m_list_lock.Initialize();
	m_allocation_lock.Initialize();
}

MessageQueue::~MessageQueue() {
	m_allocator.Clear();
	free(m_queue_memory);

	m_list_lock.Terminate();
	m_allocation_lock.Terminate();
}

bool MessageQueue::Write(const void* data, unsigned int bytes) {

	void* message_memory = NULL;

	{
		Base::MutexAutoLock lock(m_allocation_lock);
		message_memory = m_allocator.Allocate(sizeof(Message) + bytes);

		if(NULL == message_memory) {
			// can't allocate more data.
			return false;
		}

		m_total_message_bytes += bytes;
	}

	Message* message = new(message_memory)Message();

	message->size = bytes;
	message->next = nullptr;
	memcpy(message->data, data, bytes);

	{
		Base::MutexAutoLock lock(m_list_lock);
		// push_back message.
		if(!m_message_list) {
			m_message_list = message;
		} else {
			Message* msg = m_message_list;
			while(msg->next) {
				msg = msg->next;
			}
			msg->next = message;
		}
	}

	return true;
}

Result MessageQueue::Read(void* data, unsigned int max_length, streamsize* read) {

	Message* message = NULL;

	{
		Base::MutexAutoLock lock(m_list_lock);
		// pop front
		message = m_message_list;

		if(NULL == message) {
			// no more data to read.
			*read = 0;
			return Result::RS_EMPTY;
		}

		if(message->size > max_length) {
			*read = message->size;
			return Result::RS_BUFFER_TOO_SMALL;
		}

		if(m_message_list){
			m_message_list = m_message_list->next;
		}
	}

	unsigned int read_size = message->size;

	memcpy(data, message->data, read_size);

	message->~Message();

	{
		Base::MutexAutoLock lock(m_allocation_lock);
		m_allocator.Deallocate(message);
		m_total_message_bytes -= read_size;
	}

	*read = read_size;
	return Result::RS_SUCCESS;
}

unsigned int MessageQueue::NextMessageSize() const {
	{
		Base::MutexAutoLock lock(m_list_lock);
		if(m_message_list) {
			return m_message_list->size;
		}
	}
	return 0;
}

} // namespace Link
