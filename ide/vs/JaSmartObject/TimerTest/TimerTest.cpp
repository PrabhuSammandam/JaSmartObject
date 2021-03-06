#include <vector>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <thread>
#include <Windows.h>
#include "tp_timer.hpp"

int main()
{
	static LARGE_INTEGER frequency = { 0 };

	QueryPerformanceFrequency(&frequency);

	LARGE_INTEGER count = { 0 };
	QueryPerformanceCounter(&count);

	int retval = 0;

	int timercount = 10;
	int triggercount = 0;
	std::vector<ThreadPoolTimer::Timer*> timers;

	for (int i = 0; i < timercount; i++)
	{
		ThreadPoolTimer::Timer *timer = new ThreadPoolTimer::Timer([i, &triggercount]() {
			triggercount++;
			std::cout << std::setw(2) << "timer " << i << " count=" << std::setw(5) << triggercount << std::endl;
		});

		timer->set_interval(std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::seconds(5)));
		timer->set_enabled(true);
		timers.push_back(timer);

		std::this_thread::sleep_for(std::chrono::milliseconds(10));
	}

	while (1)
	{
		Sleep(1);
	}

	ThreadPoolTimer::Timer::stopRunLoop();

	return retval;
}

