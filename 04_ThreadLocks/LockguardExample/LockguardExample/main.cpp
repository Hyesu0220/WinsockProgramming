#include <iostream>
#include <thread>
#include <chrono>
#include <mutex>
#include <vector>

using namespace std;

#define MAX_ADD_THREAD_NUMBER 10

static int Count = 0;
mutex Count_Mutex;

void Add(thread::id ID)
{
	std::lock_guard<std::mutex> locker(Count_Mutex);
	Count++;
	cout << "AddThread (" << ID << ") : " << Count << endl;
}

void AddThread()
{
	while (true)
	{
		Add(this_thread::get_id());
		this_thread::sleep_for(chrono::seconds(1));
	}
}

int main()
{
	vector<thread> addthreads;

	for (int i = 0; i < MAX_ADD_THREAD_NUMBER; i++)
	{
		cout << "Create Add Thread" << endl;
		thread temp_add_thread(AddThread);
		addthreads.push_back(move(temp_add_thread));
		cout << "Created Add Thread" << endl;
	}

	for (thread& t : addthreads)
	{
		if (t.joinable())
			t.join();
	}
}