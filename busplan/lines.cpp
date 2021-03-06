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

    std::stable_sort(rv.begin(), rv.end());
    rv.erase(std::unique(rv.begin(), rv.end()), rv.end());

    return rv;
}
