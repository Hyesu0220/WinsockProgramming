#include <iostream>
#include <thread>
#include <chrono>
#include <mutex>
#include "Semaphore.h"
#include <vector>

using namespace std;

#define MAX_ADD_THREAD_NUMBER 10

static int Count = 0;
Semaphore Count_Semaphore(5);

void Add(thread::id ID)
{
	cout << "Add ThreadID(" << ID << ") " << endl;
	Count_Semaphore.Wait();
	for (int i = 0; i < 10; i++)
	{
		Count++;
		cout << "Add() : " << Count << endl;
	}	
	Count_Semaphore.Notify();
}

void Minus(thread::id ID)
{
	cout << "Minus ThreadID(" << ID << ") " << endl;
	Count_Semaphore.Wait();
	for (int i = 0; i < 10; i++)
	{
		Count--;
		cout << "Minus() : " << Count << endl;
	}	
	Count_Semaphore.Notify();
}

void AddWorkThread()
{
	while (true)
	{
		Add(this_thread::get_id());
		this_thread::sleep_for(chrono::seconds(1));
	}
}

void MinusWorkThread()
{
	while (true)
	{
		Minus(this_thread::get_id());
		this_thread::sleep_for(chrono::seconds(1));
	}
}

int main()
{
	vector<thread> threads;

	for (int i = 0; i < MAX_ADD_THREAD_NUMBER; i++)
	{
		cout << "Create Worker Thread" << endl;
		if (i % 2 == 0)
		{
			thread temp_add_thread(AddWorkThread);
			threads.push_back(move(temp_add_thread));
		}
		else
		{
			thread temp_minus_thread(MinusWorkThread);
			threads.push_back(move(temp_minus_thread));
		}		
		cout << "Created Worker Thread" << endl;
	}

	for (thread& t : threads)
	{
		if (t.joinable())
			t.join();
	}
}