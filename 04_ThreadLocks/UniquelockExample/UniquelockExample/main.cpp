#include <iostream>
#include <thread>
#include <chrono>
#include <mutex>
#include <vector>

using namespace std;

#define MAX_ADD_THREAD_NUMBER 10

static int Count = 0;
mutex Count_AddMutex;
mutex Count_MinusMutex;

void Add()
{
	Count++;
	cout << "Add() : " << Count << endl;
}

void Minus()
{
	Count--;
	cout << "Minus() : " << Count << endl;
}

void AddMInus(thread::id ID)
{
	std::unique_lock<std::mutex> add_locker(Count_AddMutex, std::defer_lock);
	std::unique_lock<std::mutex> minus_locker(Count_MinusMutex, std::defer_lock);

	std::lock(add_locker, minus_locker);
	cout << "ThreadID(" << ID << ") " << endl;
	Add();
	Minus();
}

void WorkThread()
{
	while (true)
	{
		AddMInus(this_thread::get_id());
		this_thread::sleep_for(chrono::seconds(1));
	}
}

int main()
{
	vector<thread> addminusthreads;

	for (int i = 0; i < MAX_ADD_THREAD_NUMBER; i++)
	{
		cout << "Create Worker Thread" << endl;
		thread temp_add_minus_thread(WorkThread);
		addminusthreads.push_back(move(temp_add_minus_thread));
		cout << "Created Worker Thread" << endl;
	}

	for (thread& t : addminusthreads)
	{
		if (t.joinable())
			t.join();
	}
}