#include "Timer.h"
using namespace lun;

TimePoint::TimePoint(TP time, std::string n) : point(time), name(n) {}

TP Timer::getTime() { return std::chrono::high_resolution_clock::now(); }

Timer::Timer() {
	start = end = getTime();
	isActive = true;
}

void Timer::lap(std::string lap) {
	if (!isActive) return;
	timePoints.push_back(TimePoint(getTime(), lap));
}

void Timer::stop() {
	end = getTime();
	isActive = false;
}

void Timer::print() {
	if (isActive) printf("Timer started %f seconds ago and has %u laps\n", getDuration(), (u32)timePoints.size());
	else printf("Timer started %f seconds ago, has %u laps and ended after %f seconds\n", getDuration(), (u32)timePoints.size(), duration(start, end));

	for (u32 i = 0; i < timePoints.size(); ++i)
		printf("Lap %u (%s)\tstart: %fs\tlast lap: %fs\n", i + 1, timePoints[i].name.c_str(), duration(start, timePoints[i].point), i == 0 ? duration(start, timePoints[i].point) : duration(timePoints[i - 1].point, timePoints[i].point));

	if (!isActive)
		printf("Lap end: %fs\tlast lap: %fs\n", duration(start, end), (timePoints.size() != 0 ? duration(timePoints[timePoints.size() - 1].point, end) : duration(start, end)));
}

f64 Timer::duration(TP t0, TP t1) { return std::chrono::duration<f64>(t1 - t0).count(); }
f64 Timer::count(f64 previous) { return getDuration() - previous; }

f64 Timer::getDuration() { return duration(start, getTime()); }