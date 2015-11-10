#include <algorithm>

#include "lines.hpp"

StopSet Lines::getStopSet() const {
    StopSet rv;
    for (const auto& linep: lines_) {
        auto    lstopSet = linep.second.getStopSet();
        rv.insert(lstopSet.cbegin(), lstopSet.cend());
    }
    for (const auto& walkingTimep: walkingTimes_) {
        rv.insert(walkingTimep.first.first);
        rv.insert(walkingTimep.first.second);
    }
    return rv;
}

StepsLines Lines::getForwardStepsLines() const {
    StepsLines  rv;
    for (const auto& linep: lines_) {
        rv.emplace_back(linep.first, std::move(linep.second.getForwardStepsRoutes()));
    }
    return rv;
}

StepsLines Lines::getBackwardStepsLines() const {
    StepsLines  rv;
    for (const auto& linep: lines_) {
        rv.emplace_back(linep.first, std::move(linep.second.getBackwardStepsRoutes()));
    }
    return rv;
}

TimeLine Lines::getStopTimes(Day day, const Stop& stop) const {
    TimeLine    rv;
    for (const auto& linep: lines_) {
        auto    tl = linep.second.getStopTimes(day, stop);
        rv.insert(rv.end(), tl.cbegin(), tl.cend());
    }
    auto    segmentTimes = getSegmentTimes(wakingTimes_, stop);
    for (const auto& segmentTime: segmentTimes) {
        auto    next = stop == segmentTime.first.first ? segmentTime.first.second : segmentTime.first.first;
        auto    difTime = segmentTime.second;
        auto    times = getStopTimes(day, next);
        for (auto& time: times) {
            time += difTime;
        }
        rv.insert(rv.end(), times.cbegin(), times.cend());
    }

    std::sort(rv.begin(), rv.end());
    rv.erase(std::unique(rv.begin(), rv.end()), rv.end());

    return rv;
}

TimeSteps Lines::getStopTimesByRoute(Day day, const Stop& stop) const {
    TimeSteps    rv;
    for (const auto& linep: lines_) {
        auto    timesByRouteName = linep.second.getStopTimesByRouteName(day, stop);
        for (const auto& timeByRouteName: timesByRouteName) {
            rv.emplace_back(linep.first, timeByRouteName.first, timeByRouteName.second);
        }
    }

    std::sort(rv.begin(), rv.end());

    return rv;
}

TimeSteps Lines::getBoundArriveTimesByRoute(Day day, const Stop& to, Time arrive) const {
    TimeSteps    rv;
    for (const auto& linep: lines_) {
        auto    timesByRouteName = linep.second.getBoundArriveTimesByRouteName(day, to, arrive);
        for (const auto& timeByRouteName: timesByRouteName) {
            rv.emplace_back(linep.first, timeByRouteName.first, timeByRouteName.second);
        }
    }
    return rv;
}

