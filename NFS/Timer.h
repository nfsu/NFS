#pragma once

#include "Types.h"
#include <chrono>

namespace oi {

	typedef std::chrono::high_resolution_clock::time_point CTP;

	struct TimePoint {
		CTP point;
		std::string name;

		TimePoint(CTP time, std::string n);
	};

	class Timer {

	public:

		Timer();
		void lap(std::string lap);
		void stop();

		void print();
		f64 getDuration();

		f64 count(f64 previous);

		static CTP getTime();

	private:

		static f64 duration(CTP t0, CTP t1);

		std::vector<TimePoint> timePoints;
		CTP start;
		CTP end;
		bool isActive;
	};
}