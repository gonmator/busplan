#pragma once
#ifndef ROUTE_HPP
#define ROUTE_HPP

#include <algorithm>
#include <array>
#include <cassert>
#include <iterator>
#include <map>
#include <vector>

#include "algorithm.hpp"
#include "day.hpp"
#include "schedule.hpp"
#include "stop.hpp"

using Segment = std::pair<Stop, Stop>;
using Segments = std::vector<Segment>;

class Route {
public:

    const std::string& description() const {
        return description_;
    }
    void description(std::string desc) {
        description_ = std::move(desc);
    }
    Schedule& schedule(Day day) {
        assert(day < 7);
        return schedules_[day];
    }
    const Stops& stops() const {
        return stops_;
    }

    std::string getPlatform(const Stop& stop) const {
        auto    it = platforms_.find(stop);
        if (it != platforms_.end()) {
            return it->second;
        }
        return std::string{};
    }

    Segments getForwardSteps() const {
        return getSteps(stops_.cbegin(), stops_.cend());
    }
    Segments getBackwardSteps() const {
        return getSteps(stops_.crbegin(), stops_.crend());
    }

    void setDescription(std::string description) {
        description_ = std::move(description);
    }
    const std::string getDescription() const {
        return description_;
    }

    Stop& addStop(const std::string& sstr) {
        stops_.push_back(Stop{sstr});
        return stops_.back();
    }
    void addPlatform(const std::string& sstr, const std::string& pstr) {
        const auto& stop = *std::find(stops_.cbegin(), stops_.cend(), sstr);
        platforms_[stop] = pstr;
    }

    const Stop& getNextStop(const Stop& stop) const {
        auto    nextIt = std::next(std::find(stops_.cbegin(), stops_.cend(), stop));
        while (*nextIt == stop) {
            ++nextIt;
        }
        return *nextIt;
    }

    TimeLine getStopTimes(Day day, const Stop& stop) const {
        return schedules_[day].getStopTimes(stopIndex(stop));
    }

    Time getArriveTime(Day day, const Stop& from, Time leave, const Stop& to) const {
        assert(day < 7);
        return schedules_.at(day).getArriveTime(stopIndex(from), leave, stopIndex(to));
    }
    Time getLeaveTime(Day day, const Stop& from, const Stop& to, Time arrive) const {
        assert(day < 7);
        return schedules_.at(day).getLeaveTime(stopIndex(from), stopIndex(to), arrive);
    }
    Time getBoundArriveTime(Day day, const Stop& to, Time arrive) const {
        assert(day < 7);
        return schedules_.at(day).getBoundArriveTime(stopIndex(to), arrive);
    }

private:
    template <typename InputIt>
    static Segments getSteps(InputIt first, InputIt last) {
        Segments   rv;
        setPairs(first, last, std::back_inserter(rv));
        return rv;
    }
    size_t stopIndex(const std::string& stop) const {
        return std::find(stops_.cbegin(), stops_.cend(), stop) - stops_.cbegin();
    }

    std::string                 description_;
    Stops                       stops_;
    std::map<Stop, std::string> platforms_;
    std::array<Schedule, 7>     schedules_;
};

#endif // ROUTE_HPP
