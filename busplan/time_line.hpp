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

#endif // TIME_LINE_HPP
