#include "TimedLoops.h"

TimedLoops::TimedLoops(const unsigned long period) {
	this->_period = period;
	_num_functions = 0;
};

TimedLoops::~TimedLoops(void) {
	free(_function_list);
};

const bool TimedLoops::check_time(const unsigned long time_last, const unsigned long period) {
	return ((millis() - time_last) > period) ? true : false;
};

void TimedLoops::add_function(const void* func) {
	func_t temp = (func_t)calloc(_num_functions + 1, sizeof(func_t));

	if (_num_functions > 0) {
		memmove(temp, _function_list, _num_functions * sizeof(func_t));
	}

	// Increase memory allocation for the function list
	_num_functions += 1;
	_function_list = (func_t*)realloc(_function_list, _num_functions * sizeof(func_t));
	
	// Copy new function to list
	memcpy(_function_list[_num_functions - 1], func, sizeof(func_t));
};

void TimedLoops::run(void) {
	if (!check_time(_time_last, _period)) return;
	for (int x = 0; x < _num_functions; x++) _function_list[x]();
	_time_last = millis();
};