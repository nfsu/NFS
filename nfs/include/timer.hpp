#pragma once
#include "generic.hpp"
#include <chrono>
#include <sstream>

namespace oi {

	typedef std::chrono::high_resolution_clock::time_point CTP;

	struct TimePoint {

		CTP point;
		String name;

		TimePoint(CTP time, const String &n);
	};

	class Timer {

	public:

		Timer();
		void lap(const String &lap);
		void stop();
		void print();
		f64 getDuration();
		f64 count(f64 previous);

		static CTP getTime();

	private:

		f64 duration(CTP t0, CTP t1);

		List<TimePoint> timePoints;
		CTP start;
		CTP end;
		bool isActive;
	};
}