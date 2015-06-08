#include <iterator>

#include "schedule.hpp"


TimeLine Schedule::getStopTimes(size_t stopIx) const {
    if (stopIx >= maxStopCount_) {
        throw std::out_of_range{"stop index out of range in schedule"};
    }

    TimeLine    rv;
    for (const auto& fragmentp: fragments_) {
        auto    fromIx = getStopIndex(fragmentp.first);
        if (isStopInFragment(fragmentp.first, stopIx)) {
            auto    fragmenttl = fragmentp.second.getStopTimes(stopIx - fromIx);
            std::move(fragmenttl.begin(), fragmenttl.end(), std::back_inserter(rv));
        }
    }
    return reduceTimeLine(rv);
}

Time Schedule::getArriveTime(size_t fromIx, Time leave, size_t toIx) const {
    TimeLine    arrives;
    for (const auto& fragmentp: fragments_) {
        auto    startIx = getStopIndex(fragmentp.first);
        if (isStopInFragment(fragmentp.first, fromIx) && isStopInFragment(fragmentp.first, toIx)) {
            auto    arrive_result = fragmentp.second.findArriveTime(fromIx - startIx, leave, toIx - startIx);
            if (arrive_result.second) {
                arrives.push_back(arrive_result.first);
            }
        }
    }
    return reduceTimeLine(arrives).front();
}


