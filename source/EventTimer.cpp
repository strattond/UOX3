//

#include "EventTimer.hpp"

#include <iostream>
using namespace std::string_literals;
//=========================================================
EventTimer::EventTimer() {
	_now = std::chrono::high_resolution_clock::now();
}
//=========================================================

auto EventTimer::elapsed() -> long long {
	auto delta =std::chrono::high_resolution_clock::now()-_now ;
	_now =std::chrono::high_resolution_clock::now();
	return std::chrono::duration_cast<std::chrono::milliseconds>(delta).count();
}

//=========================================================
auto EventTimer::output(const std::string &label = "Delta from last call")->void {
	std::cout <<label <<" - "s <<elapsed()<<"(ms)\n"s;
}
