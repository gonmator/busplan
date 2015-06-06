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
    void removeRoute(const std::string& rstr) {
        routes_.erase(routes_.find(rstr));
    }

    RouteNames getRouteNames() const {
        return getKeyVector(routes_);
    }
    std::string getPlatform(const RouteName& routen, const Stop& stop) const {
        return routes_.at(routen).getPlatform(stop);
    }
    StopSet getStopSet() const;
    StepsRoutes getForwardStepsRoutes() const;
    StepsRoutes getBackwardStepsRoutes() const;
    TimeLine getStopTimes(Day day, const RouteName& routen, const Stop& stop) const {
        return routes_.at(routen).getStopTimes(day, stop);
    }
    TimeLine getStopTimes(Day day, const Stop& stop) const;
    Time getArriveTime(Day day, const RouteName& routen, const Stop& from, Time leave, const Stop& to) const {
        return routes_.at(routen).getArriveTime(day, from, leave, to);
    }
    std::string getRouteDescription(const RouteName& routen) const {
        return routes_.at(routen).description();
    }

private:
    std::map<RouteName, Route>  routes_;
};


#endif // LINE_HPP
