#pragma once
#include "Types.h"
#include <chrono>

//Classes from (old) lun engine
namespace lun {
	typedef std::chrono::high_resolution_clock::time_point TP;

	struct TimePoint {
		TP point;
		std::string name;

		TimePoint(TP time, std::string n);
	};

	class Timer {

	public:

		Timer();
		void lap(std::string lap);
		void stop();

		void print();
		f64 getDuration();

		f64 count(f64 previous);

		static TP getTime();

	private:

		static f64 duration(TP t0, TP t1);

		std::vector<TimePoint> timePoints;
		TP start;
		TP end;
		bool isActive;
	};
}