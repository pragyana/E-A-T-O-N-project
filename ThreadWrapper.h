/*
 *  @file: ThreadWrapper.h
 *  @author: Ryan Dutton
 *  @date: 1/3/2021
 *
 */


#include <thread>

class ThreadWrapper
{
	public:
	using RAIIAction = void (std::thread::*)();
	ThreadWrapper(std::thread&& _t, RAIIAction a)
		: t(std::move(_t)), action(a) {}

	//move constructor
	ThreadWrapper(ThreadWrapper&& orig) : t(std::move(orig.t)), action(orig.action)
	{
	}

	//Hide copy constructor, and assign operators
	ThreadWrapper(const ThreadWrapper&) = delete;
	ThreadWrapper& operator=(const ThreadWrapper&) = delete;
	ThreadWrapper& operator=(ThreadWrapper&&) = delete;

	//Join the thread in the destructor
	~ThreadWrapper() {
		if (t.joinable()) { (t.*action)(); }
	}

	private:
	RAIIAction action;
	std::thread t;
};



