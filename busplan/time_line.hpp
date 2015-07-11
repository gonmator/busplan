#pragma once
#ifndef TIME_LINE_HPP
#define TIME_LINE_HPP

#include <algorithm>
#include <chrono>
#include <map>
#include <string>
#include <vector>

#include "stop.hpp"
#include "time.hpp"

struct DifTimeLine {
    Stop                    from;
    std::vector<DifTime>    durations;
};

using DifTimeLines = std::map<std::string, DifTimeLine>;
using TimeLine = std::vector<Time>;

inline bool operator<(const TimeLine& a, const TimeLine& b) {
    return std::lexicographical_compare(a.cbegin(), a.cend(), b.cbegin(), b.cend());
}

TimeLine applyDurations(const DifTimeLine& dtline, Time start);

inline TimeLine& reduceTimeLine(TimeLine& timeline) {
    std::sort(timeline.begin(), timeline.end());
    timeline.erase(std::unique(timeline.begin(), timeline.end()), timeline.end());
    return timeline;
}

#endif // TIME_LINE_HPP
