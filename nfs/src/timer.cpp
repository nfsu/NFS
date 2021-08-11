#include "timer.hpp"
using namespace oi;

TimePoint::TimePoint(CTP time, const String &n) : point(time), name(n) {}

Timer::Timer() {
	start = end = getTime();
	isActive = true;
}

void Timer::lap(const String &lap) {

	if (!isActive) 
		return;

	timePoints.push_back(TimePoint(getTime(), lap));
}

void Timer::stop() {
	end = getTime();
	isActive = false;
}

void Timer::print() {

	std::stringstream ss;

	if (isActive) 
		ss << "Timer started " << getDuration() << " seconds ago and has " << timePoints.size() << " laps\n";

	else 
		ss << "Timer started " << getDuration() << " seconds ago, has " << timePoints.size() << 
		" laps and ended after " << duration(start, end) << " seconds\n";

	for (u32 i = 0; i < timePoints.size(); ++i)
		ss << "Lap " << (i + 1) << ": " << timePoints[i].name << "; time since start " << 
		duration(start, timePoints[i].point) << "s, time since last time point: " << 
		(i == 0 ? f32(duration(start, timePoints[i].point)) : f32(duration(timePoints[i - 1].point, timePoints[i].point))) << 
		"s\n";

	std::printf("%s", ss.str().c_str());
}

f64 Timer::getDuration() { return duration(start, getTime()); }

f64 Timer::count(f64 previous) { return getDuration() - previous; }

CTP Timer::getTime() { return std::chrono::high_resolution_clock::now(); }

f64 Timer::duration(CTP t0, CTP t1) { return std::chrono::duration<f64>(t1 - t0).count(); }