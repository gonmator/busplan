#pragma once
#ifndef TIME_STEP_HPP
#define TIME_STEP_HPP

#include <vector>

#include "routeid.hpp"
#include "time.hpp"


struct TimeStep {
    TimeStep(LineName lineName, const RouteName routeName, Time time):
        TimeStep(RouteId{std::move(lineName), std::move(routeName)}, time) {
    }
    TimeStep(RouteId routeid, Time time): routeid{std::move(routeid)}, time{time} {
    }
    TimeStep(Time time): routeid{}, time{time} {
    }
    TimeStep() = default;
    TimeStep(const TimeStep&) = default;
    TimeStep(TimeStep&&) = default;
    TimeStep& operator=(const TimeStep&) = default;
    TimeStep& operator=(TimeStep&&) = default;

    RouteId routeid;
    Time    time;
};

inline bool operator <(const TimeStep& tstepa, const TimeStep& tstepb) {
    return tstepa.time < tstepb.time || (tstepa.time == tstepb.time && tstepa.routeid < tstepb.routeid);
}


using TimeSteps = std::vector<TimeStep>;


#endif // TIME_STEP_HPP
