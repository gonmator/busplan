#pragma once
#ifndef LINES_HPP
#define LINES_HPP

#include <map>
#include <string>
#include <vector>

#include "line.hpp"
#include "stop.hpp"
#include "walking.hpp"

using LineName = std::string;
using LineNames = std::vector<LineName>;
using StepsByLine = std::pair<LineName, StepsRoutes>;
using StepsLines = std::vector<StepsByLine>;

struct RouteId {
    RouteId(): linen{}, routen{} {};
    RouteId(LineName ln, RouteName rn): linen{std::move(ln)}, routen{std::move(rn)} {}

    LineName    linen;
    RouteName   routen;
};
inline bool operator ==(const RouteId& rida, const RouteId& ridb) {
    return rida.linen == ridb.linen && rida.routen == ridb.routen;
}
inline bool operator !=(const RouteId& rida, const RouteId& ridb) {
    return !(rida == ridb);
}

const RouteId   walkingRouteId{"__walking__", "__"};

class Lines {
public:
    Line& addLine(const LineName& lname) {
        return lines_[lname];
    }
    WalkingTimes& walkingTimes() {
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
    StopSet getStopSet() const {
        StopSet rv;
        for (const auto& linep: lines_) {
            auto    lstopSet = linep.second.getStopSet();
            rv.insert(lstopSet.cbegin(), lstopSet.cend());
        }
        return rv;
    }
    StepsLines getForwardStepsLines() const {
        StepsLines  rv;
        for (const auto& linep: lines_) {
            rv.emplace_back(linep.first, std::move(linep.second.getForwardStepsRoutes()));
        }
        return rv;
    }
    StepsLines getBackwardStepsLines() const {
        StepsLines  rv;
        for (const auto& linep: lines_) {
            rv.emplace_back(linep.first, std::move(linep.second.getBackwardStepsRoutes()));
        }
        return rv;
    }

    TimeLine getStopTimes(Route::Day day, const RouteId& routeid, const Stop& stop) const {
        return lines_.at(routeid.linen).getStopTimes(day, routeid.routen, stop);
    }

private:
    std::map<LineName, Line>    lines_;
    WalkingTimes                walkingTimes_;
};

#endif // LINES_HPP
