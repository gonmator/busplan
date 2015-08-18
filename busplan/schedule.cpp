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
    Time    arrive{plusInf};
    for (const auto& fragmentp: fragments_) {
        auto    startIx = getStopIndex(fragmentp.first);
        if (isStopInFragment(fragmentp.first, fromIx) && isStopInFragment(fragmentp.first, toIx)) {
            auto    arrive_result = fragmentp.second.findArriveTime(fromIx - startIx, leave, toIx - startIx);
            if (arrive_result.second && arrive_result.first < arrive) {
                arrive = arrive_result.first;
            }
        }
    }
    assert(arrive < plusInf);
    return arrive;
}

Time Schedule::getLeaveTime(size_t fromIx, size_t toIx, Time arrive) const {
    Time    leave{minusInf};
    for (const auto& fragmentp: fragments_) {
        auto    startIx = getStopIndex(fragmentp.first);
        if (isStopInFragment(fragmentp.first, fromIx) && isStopInFragment(fragmentp.first, toIx)) {
            auto    leave_result = fragmentp.second.findLeaveTime(fromIx - startIx, toIx - startIx, arrive);
            if (leave_result.second && leave_result.first > leave) {
                leave = leave_result.first;
            }
        }
    }
    return leave;
}

Time Schedule::getBoundArriveTime(size_t toIx, Time arrive) const {
    Time    boundArrive{minusInf};
    for (const auto& fragmentp: fragments_) {
        auto    startIx = getStopIndex(fragmentp.first);
        if (isStopInFragment(fragmentp.first, toIx)) {
            auto    arrive_result = fragmentp.second.findBoundArriveTime(toIx - startIx, arrive);
            if (arrive_result.second && arrive_result.first > boundArrive) {
                boundArrive = arrive_result.first;
            }
        }
    }
    return boundArrive;
}
