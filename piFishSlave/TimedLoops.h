#ifndef _TIMEDLOOPS_H
#define _TIMEDLOOPS_H

#define TIMEDLOOPS_VERSION  "0.1"

#if defined(ARDUINO) && ARDUINO >= 100
#include "arduino.h"
#else
#include "WProgram.h"
#endif

typedef void(*func_t)(); // pointer to function with no args and void return

class TimedLoops
{
public:
	TimedLoops(const unsigned long period);
	~TimedLoops(void);
	void add_function(const void* func);
	void run(void);
protected:
	const static bool check_time(const unsigned long time_last, const unsigned long period);
private:
	unsigned long _period;
	unsigned long _time_last;
	unsigned long _num_functions;
	func_t*       _function_list;
};

#endif
