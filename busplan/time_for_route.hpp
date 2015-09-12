#pragma once
#ifndef TIME_FOR_ROUTE_HPP
#define TIME_FOR_ROUTE_HPP

#include <vector>

#include "routeid.hpp"
#include "time.hpp"


struct TimeForRoute {
    TimeForRoute(LineName lineName, const RouteName routeName, Time time):
        TimeForRoute(RouteId{std::move(lineName), std::move(routeName)}, time) {
    }
    TimeForRoute(RouteId routeid, Time time): routeid{std::move(routeid)}, time{time} {
    }
    TimeForRoute(Time time): routeid{}, time{time} {
    }
    TimeForRoute() = default;
    TimeForRoute(const TimeForRoute&) = default;
    TimeForRoute(TimeForRoute&&) = default;
    TimeForRoute& operator=(const TimeForRoute&) = default;
    TimeForRoute& operator=(TimeForRoute&&) = default;

    RouteId routeid;
    Time    time;
};

inline bool operator <(const TimeForRoute& tfra, const TimeForRoute& tfrb) {
    return tfra.time < tfrb.time || (tfra.time == tfrb.time && tfra.routeid < tfrb.routeid);
}


using TimeForRoutes = std::vector<TimeForRoute>;


#endif // TIME_FOR_ROUTE_HPP
