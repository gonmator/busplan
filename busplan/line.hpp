#pragma once
#ifndef LINE_HPP
#define LINE_HPP

#include <initializer_list>
#include <map>
#include <string>
#include <vector>

#include "route.hpp"

using RouteName = std::string;
using RouteNames = std::vector<RouteName>;
using StepsByRoute = std::pair<RouteName, Steps>;
using StepsRoutes = std::vector<StepsByRoute>;

class Line {
public:

    Route& addRoute(const std::string& rstr) {
        return routes_[rstr];
    }

    RouteNames getRouteNames() const {
        return getKeyVector(routes_);
    }
    std::string getPlatform(const RouteName& routen, const Stop& stop) const {
        return routes_.at(routen).getPlatform(stop);
    }
    StopSet getStopSet() const {
        StopSet rv;
        for (const auto& routep: routes_) {
            const auto& rstops = routep.second.stops();
            rv.insert(rstops.cbegin(), rstops.cend());
        }
        return rv;
    }
    StepsRoutes getForwardStepsRoutes() const {
        StepsRoutes rv;
        for (const auto& routep: routes_) {
            rv.emplace_back(routep.first, std::move(routep.second.getForwardSteps()));
        }
        return rv;
    }
    StepsRoutes getBackwardStepsRoutes() const {
        StepsRoutes rv;
        for (const auto& routep: routes_) {
            rv.emplace_back(routep.first, std::move(routep.second.getBackwardSteps()));
        }
        return rv;
    }

    TimeLine getStopTimes(Route::Day day, const RouteName& routen, const Stop& stop) const {
        return routes_.at(routen).getStopTimes(day, stop);
    }

private:
    std::map<RouteName, Route>  routes_;
};


#endif // LINE_HPP
