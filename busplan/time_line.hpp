#pragma once
#ifndef TIME_LINE_HPP
#define TIME_LINE_HPP

#include <chrono>
#include <map>
#include <string>
#include <vector>

#include "time.hpp"

using DifTimeLine = std::vector<DifTime>;
using DifTimeLines = std::map<std::string, DifTimeLine>;
using TimeLine = std::vector<Time>;

inline TimeLine applyTimeZone(const DifTimeLine& dtline, Time::rep tzone) {
    TimeLine    rv;
    auto    hs = std::chrono::hours{tzone};
    for (const auto& t: dtline) {
        rv.emplace_back(hs + t);
    }
    return rv;
}

inline TimeLine applyDurations(const DifTimeLine& dtline, Time start, size_t stopCount) {
    TimeLine    rv{start};
    for (auto duration: dtline) {
        start += duration;
        rv.push_back(start);
    }
    start += std::chrono::hours{24};
    while (rv.size() < stopCount) {
        rv.push_back(start);
    }
    return rv;
}

#endif // TIME_LINE_HPP
