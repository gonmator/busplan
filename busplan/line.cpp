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

StepsRoutes Line::getForwardStepsRoutes() const {
    StepsRoutes rv;
    for (const auto& routep: routes_) {
        rv.emplace_back(routep.first, std::move(routep.second.getForwardSteps()));
    }
    return rv;
}

StepsRoutes Line::getBackwardStepsRoutes() const {
    StepsRoutes rv;
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
    std::stable_sort(rv.begin(), rv.end());
    rv.erase(std::unique(rv.begin(), rv.end()), rv.end());

    return rv;
}


