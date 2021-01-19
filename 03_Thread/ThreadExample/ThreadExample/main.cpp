#include <iostream>
#include <chrono>
#include <thread>

using std::thread;

struct Vector
{
	float x = 0.0f;
	float y = 0.0f;
	float z = 0.0f;
};

void PrintVector(const Vector* InVector)
{
	while (true)
	{
		std::cout << "Running Thread() " << std::this_thread::get_id() << " (" << InVector->x << ", " << InVector->y << ", " << InVector->z << ")" << std::endl;
		std::this_thread::sleep_for(std::chrono::milliseconds(1000));
	}
}

int main()
{
	std::cout << "Number of processor : " << std::thread::hardware_concurrency() << std::endl;
	Vector vector_1 = { 20.0f, 100.5f, -20.5f };
	Vector vector_2 = { -20.0f, 300.5f, -10.7f };
	Vector vector_3 = { 57.8f, 104.5f, 72.7f };

	thread th1(PrintVector, &vector_1);
	thread th2(PrintVector, &vector_2);

	// Lambda ½Ä Ç¥Çö
	thread th3([](const Vector* in_vector) {
		while (true)
		{
			std::cout << "Running Thread() " << std::this_thread::get_id() << " (" << in_vector->x << ", " << in_vector->y << ", " << in_vector->z << ")" << std::endl;
			std::this_thread::sleep_for(std::chrono::milliseconds(1000));
		}
	}, &vector_3);

	th1.join();
	th2.join();
	th3.join();
}

/**
 * @see [Thread Priority](https://stackoverflow.com/questions/18884510/portable-way-of-setting-stdthread-priority-in-c11)
 * @see [Thread Reference](http://www.cplusplus.com/reference/thread/thread/)
 */