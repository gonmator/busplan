#pragma once
#ifndef LINES_HPP
#define LINES_HPP

#include <map>
#include <string>
#include <vector>

#include "line.hpp"
#include "routeid.hpp"
#include "stop.hpp"
#include "time_step.hpp"
#include "walking.hpp"

using StepsByLine = std::pair<LineName, RouteNameSegmentsList>;
using StepsLines = std::vector<StepsByLine>;


const RouteId   walkingRouteId{"__walking__", "__"};


class Lines {
public:
    Line& addLine(const LineName& lname) {
        return lines_[lname];
    }
    void removeLine(const LineName& lname) {
        lines_.erase(lines_.find(lname));
    }

    WalkingSegmentTimes& walkingTimes() {
        return walkingTimes_;
    }

    LineNames getLineNames() const {
        return getKeyVector(lines_);
    }
    RouteNames getRouteNames(const LineName& linen) const {
        return lines_.at(linen).getRouteNames();
    }
    std::string getPlatform(const RouteId& routeid, const Stop& stop) const {
        if (routeid == walkingRouteId) {
            return "walking";
        }
        return lines_.at(routeid.linen).getPlatform(routeid.routen, stop);
    }
    StopSet getStopSet() const;
    StepsLines getForwardStepsLines() const;
    StepsLines getBackwardStepsLines() const;

    const Stop& getNextStop(const RouteId& routeid, const Stop& stop) const {
        return getNextStop(routeid.linen, routeid.routen, stop);
    }
    const Stop& getNextStop(const LineName& linen, const RouteName& routen, const Stop& stop) const {
        return lines_.at(linen).getNextStop(routen, stop);
    }

    TimeLine getStopTimes(Day day, const RouteId& routeid, const Stop& stop) const {
        return lines_.at(routeid.linen).getStopTimes(day, routeid.routen, stop);
    }
    TimeLine getStopTimes(Day day, const Stop& stop) const;

    TimeSteps getStopTimesByRoute(Day day, const Stop& stop) const;

    Time getArriveTime(Day day, const RouteId& routeid, const Stop& from, Time leave, const Stop& to) const {
        if (routeid == walkingRouteId) {
            return ::getArriveTime(walkingTimes_, from, leave, to);
        }
        return lines_.at(routeid.linen).getArriveTime(day, routeid.routen, from, leave, to);
    }
    Time getLeaveTime(Day day, const RouteId& routeid, const Stop& from, const Stop& to, Time arrive) const {
        if (routeid == walkingRouteId) {
            return ::getLeaveTime(walkingTimes_, from, to, arrive);
        }
        return lines_.at(routeid.linen).getLeaveTime(day, routeid.routen, from, to, arrive);
    }
    TimeSteps getBoundArriveTimesByRoute(Day day, const Stop& to, Time arrive) const;

    std::string getRouteDescription(const RouteId& routeid) const {
        if (routeid == walkingRouteId) {
            return "(walking)";
        }
        if (routeid.linen.empty()) {
            return "";
        }
        return lines_.at(routeid.linen).getRouteDescription(routeid.routen);
    }

private:
    std::map<LineName, Line>    lines_;
    WalkingSegmentTimes                walkingTimes_;
};

#endif // LINES_HPP
