#pragma once
#ifndef WALKING_HPP
#define WALKING_HPP

#include <map>

#include "stop.hpp"
#include "time.hpp"
#include "time_line.hpp"

struct WalkingSegment: std::pair<Stop, Stop> {
    WalkingSegment(const Stop& pointA, const Stop& pointB): std::pair<Stop, Stop>(
        pointA < pointB ? pointA: pointB, pointA < pointB ? pointB: pointA) {
    }
    template <class U>
    WalkingSegment(U&& pointA, U&& pointB): std::pair<Stop, Stop>(
        std::forward<U>(pointA < pointB ? pointA: pointB), std::forward<U>(pointA < pointB ? pointB: pointA)) {
    }
};

using WalkingSegmentTimes = std::map<WalkingSegment, DifTime>;
using WalkingSegmentTime = std::map<WalkingSegment, DifTime>::value_type;
using WalkingSegmentList = std::vector<WalkingSegmentTime>;

inline Time getArriveTime(const WalkingSegmentTimes& walkingTimes, const Stop& from, Time leave, const Stop& to) {
    return leave + walkingTimes.at(WalkingSegment{from, to});
}

inline Time getLeaveTime(const WalkingSegmentTimes& walkingTimes, const Stop& from, const Stop& to, Time arrive) {
    return arrive - walkingTimes.at(WalkingSegment{from, to});
}

inline WalkingSegmentList getSegmentTimes(const WalkingSegmentTimes& walkingTimes, const Stop& stop) {
    WalkingSegmentList  rv{};
    std::for_each(
            walkingTimes.cbegin(), walkingTimes.cend(), [&rv, &stop](const WalkingSegmentTimes::value_type& v) {
        if (v.first.first == stop || v.first.second == stop) {
            rv.push_back(v);
        }
    });
    return rv;
}


#endif // WALKING_HPP
