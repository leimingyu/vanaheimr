/*	\file   HostReflection.h
	\author Gregory Diamos <gregory.diamos@gatech.edu>
	\date   Saturday July 16, 2011
	\brief  The header file for the HostReflection set of functions.
*/

#pragma once

// Boost Includes
#include <boost/thread.hpp>

namespace util
{

class HostReflection
{
public:
	typedef unsigned int HandlerId;

	class Message
	{
	public:
		__device__ virtual void* payload() const = 0;
		__device__ virtual size_t payloadSize() const = 0;
		__device__ virtual HostReflection::HandlerId handler() const = 0;
	};
	
	enum MessageHandler
	{
		OpenFileMessageHandler,
		TeardownFileMessageHandler,
		FileWriteMessageHandler,
		FileReadMessageHandler,
		InvalidMessageHandler = -1
	};
		
public:
	__device__ static void sendSynchronous(const Message& m);
	__device__ static void receive(Message& m);

public:
	__host__ __device__ static size_t maxMessageSize();

public:
	class Queue
	{
	public:
		__device__ Queue(char* data, size_t size);
		__device__ ~Queue();
	
	public:
		__host__ __device__ bool push(const void* data, size_t size);
		__host__ __device__ bool pull(void* data, size_t size);

	public:
		__host__ __device__ bool peek();
		__host__ __device__ size_t size() const;
	
	private:
		char*  _begin;
		char*  _end;
		char*  _head;
		char*  _tail;
		size_t _mutex;
		
	private:
		__host__ __device__ size_t _capacity() const;
		__host__ __device__ size_t _used() const;
		
	private:
		__host__ __device__ bool _lock();
		__host__ __device__ void _unlock();
		__host__ __device__ char* _read(void* data, size_t size);

	};

	enum MessageType
	{
		Synchronous,
		Asynchronous,
		Invalid,
	};

	class Header
	{
	public:
		MessageType  type;
		unsigned int threadId;
		HandlerId    handler;
	};
	
	class SynchronousHeader : public Header
	{
	public:
		void* address;
	};

private:
	class BootUp
	{
	public:
		typedef void (*MessageHandler)(const Message*);
		typedef std::unordered_map<int, MessageHandler> HandlerMap;
	
	public:
		__host__ BootUp();
		__host__ ~BootUp();

	public:
		__host__ void addHandler(int handlerId, MessageHandler handler);

	private:
		boost::thread* _thread;
		bool           _kill;
	
	private:
		HandlerMap _handlers;

	private:
		void _run();
	
	private:
		static void _run(BootUp* kill);
	};

private:
	static BootUp _booter;

};

}

// TODO remove this when we get a real linker
#include <archaeopteryx/util/implementation/HostReflection.cpp>

