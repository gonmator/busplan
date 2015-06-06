#pragma once
#ifndef WALKING_HPP
#define WALKING_HPP

#include <map>

#include "stop.hpp"
#include "time.hpp"

struct WalkingStep: std::pair<Stop, Stop> {
    WalkingStep(const Stop& pointA, const Stop& pointB): std::pair<Stop, Stop>(
        pointA < pointB ? pointA: pointB, pointA < pointB ? pointB: pointA) {
    }
    template <class U>
    WalkingStep(U&& pointA, U&& pointB): std::pair<Stop, Stop>(
        std::forward<U>(pointA < pointB ? pointA: pointB), std::forward<U>(pointA < pointB ? pointB: pointA)) {
    }
};

using WalkingTimes = std::map<WalkingStep, DifTime>;

inline Time getArriveTime(const WalkingTimes& walkingTimes, const Stop& from, Time leave, const Stop& to) {
    return leave + walkingTimes.at(WalkingStep{from, to});
}

#endif // WALKING_HPP
