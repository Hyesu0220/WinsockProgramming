#include <iostream>
#include <thread>
#include <chrono>
#include <mutex>
#include <vector>

using namespace std;

#define MAX_ADD_THREAD_NUMBER 10

static int Count = 0;
mutex Count_Mutex1, Count_Mutex2;

void AddThread()
{
	while (true)
	{
		std::lock(Count_Mutex1, Count_Mutex2);
		Count++;
		cout << "AddThread (" << this_thread::get_id() << ") : " << Count << endl;
		Count_Mutex1.unlock();
		Count_Mutex2.unlock();

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