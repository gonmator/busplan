#pragma once
#ifndef TIME_TABLE_HPP
#define TIME_TABLE_HPP

#include <vector>
#include "time_line.hpp"

using TimeTable = std::vector<TimeLine>;

bool isValid(const TimeTable& tt);
TimeTable::size_type safeStopCount(const TimeTable& tt);
TimeTable::size_type stopCount(const TimeTable& tt);
TimeTable& addTimeLine(TimeTable& tt, const TimeLine& tl);

#endif // TIME_TABLE_HPP
