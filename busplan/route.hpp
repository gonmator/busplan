#pragma once
#ifndef ROUTE_HPP
#define ROUTE_HPP

#include <algorithm>
#include <iterator>
#include <map>
#include <vector>

#include "algorithm.hpp"
#include "schedule.hpp"
#include "stop.hpp"

using Step = std::pair<Stop, Stop>;
using Steps = std::vector<Step>;

class Route {
public:
    enum class Day {
        monToFri,
        sat,
        sun
    };

    Schedule& monToFri() {
        return monToFriSchedule_;
    }
    Schedule& sat() {
        return satSchedule_;
    }
    Schedule& sun() {
        return sunSchedule_;
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

    Steps getForwardSteps() const {
        return getSteps(stops_.cbegin(), stops_.cend());
    }
    Steps getBackwardSteps() const {
        return getSteps(stops_.crbegin(), stops_.crend());
    }

    Stop& addStop(const std::string& sstr) {
        stops_.push_back(Stop{sstr});
        return stops_.back();
    }
    void addPlatform(const std::string& sstr, const std::string& pstr) {
        const auto& stop = *std::find(stops_.cbegin(), stops_.cend(), sstr);
        platforms_[stop] = pstr;
    }

    TimeLine getStopTimes(Day day, const Stop& stop) const {
        switch (day) {
        case Day::monToFri:
            return monToFriSchedule_.getStopTimes(stopIndex(stop));
        case Day::sat:
            return satSchedule_.getStopTimes(stopIndex(stop));
        case Day::sun:
            return satSchedule_.getStopTimes(stopIndex(stop));
        }
        throw std::invalid_argument("invalid schedule day");
    }

private:
    template <typename InputIt>
    static Steps getSteps(InputIt first, InputIt last) {
        Steps   rv;
        setPairs(first, last, std::back_inserter(rv));
        return rv;
    }
    size_t stopIndex(const std::string& stop) const {
        return std::find(stops_.cbegin(), stops_.cend(), stop) - stops_.cbegin();
    }

    Stops                       stops_;
    std::map<Stop, std::string> platforms_;
    Schedule                    monToFriSchedule_;
    Schedule                    satSchedule_;
    Schedule                    sunSchedule_;
};

#endif // ROUTE_HPP
