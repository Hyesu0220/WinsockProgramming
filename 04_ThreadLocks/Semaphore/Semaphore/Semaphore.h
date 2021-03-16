#pragma once

#include <mutex>
#include <condition_variable>

class Semaphore
{
private:
	std::mutex mMutex;
	std::condition_variable mCondition;
	int mCount;
public:
	Semaphore(int InCount = 0)
		:mCount(InCount)
	{	}

	inline void Notify()
	{
		std::unique_lock<std::mutex> lock(mMutex);
		mCount++;
		mCondition.notify_one();
	}

	inline void Wait()
	{
		std::unique_lock<std::mutex> lock(mMutex);

		while (mCount == 0)
		{
			mCondition.wait(lock);
		}
		mCount--;
	}
};
