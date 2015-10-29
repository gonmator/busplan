#include <algorithm>

#include "line.hpp"

StopSet Line::getStopSet() const {
    StopSet rv;
    for (const auto& routep: routes_) {
        const auto& rstops = routep.second.stops();
        rv.insert(rstops.cbegin(), rstops.cend());
    }
    return rv;
}

RouteNameSegmentsList Line::getForwardStepsRoutes() const {
    RouteNameSegmentsList rv;
    for (const auto& routep: routes_) {
        rv.emplace_back(routep.first, std::move(routep.second.getForwardSteps()));
    }
    return rv;
}

RouteNameSegmentsList Line::getBackwardStepsRoutes() const {
    RouteNameSegmentsList rv;
    for (const auto& routep: routes_) {
        rv.emplace_back(routep.first, std::move(routep.second.getBackwardSteps()));
    }
    return rv;
}

TimeLine Line::getStopTimes(Day day, const Stop& stop) const {
    TimeLine    rv;
    for (const auto& routep: routes_) {
        try {
            auto    tl = routep.second.getStopTimes(day, stop);
            rv.insert(rv.end(), tl.cbegin(), tl.cend());
        } catch (std::out_of_range&) {
            //  ignore, it is ok.
        }
    }
    std::sort(rv.begin(), rv.end());
    rv.erase(std::unique(rv.begin(), rv.end()), rv.end());

    return rv;
}

RouteNameTimeList Line::getStopTimesByRouteName(Day day, const Stop& stop) const {
    RouteNameTimeList    rv;

    for (const auto& routep: routes_) {
        try {
            auto    tl = routep.second.getStopTimes(day, stop);
            for (auto time: tl) {
                rv.emplace_back(routep.first, time);
            }
        } catch (std::out_of_range&) {
            //  ignore, it is ok.
        }
    }

    return rv;
}

RouteNameTimeList Line::getBoundArriveTimesByRouteName(Day day, const Stop& to, Time arrive) const {
    RouteNameTimeList    rv;

    for (const auto& routep: routes_) {
        try {
            auto    time = routep.second.getBoundArriveTime(day, to, arrive);
            rv.emplace_back(routep.first, time);
        } catch (std::out_of_range&) {
            //  ignore, it is ok.
        }
    }

    return rv;
}



