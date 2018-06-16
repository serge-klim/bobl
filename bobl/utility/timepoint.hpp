// Copyright (c) 2015-2018 Serge Klimov serge.klim@outlook.com

#pragma once
#include <chrono>
#include <ctime>

namespace bobl{ namespace utility {

namespace details {

inline static std::chrono::system_clock::time_point epoch()
{
	struct Init
	{
		static std::chrono::system_clock::time_point init()
		{
			auto tm = std::tm{ 0 };
			tm.tm_year = 70;
			tm.tm_mday = 1;
			return std::chrono::system_clock::from_time_t(std::mktime(&tm));
		}
	};
	static auto e = Init::init();
	return e;
}

} // namespace details


template<typename Duration, typename Rep, typename Period>
std::chrono::time_point<std::chrono::system_clock, Duration> make_timpoint(std::chrono::duration<Rep, Period> duration)
{
	return std::chrono::time_point_cast<Duration>(details::epoch() + duration);
}

template<typename ToDuration, typename TimepointDuration>
ToDuration duration_since_epoch(std::chrono::time_point<std::chrono::system_clock, TimepointDuration> timepoint)
{
	return std::chrono::duration_cast<ToDuration>(timepoint - details::epoch());
}

}/*namespace utility*/ } /*namespace bobl*/

