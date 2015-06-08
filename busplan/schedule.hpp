#pragma once
#ifndef SCHEDULE_HPP
#define SCHEDULE_HPP

#include <cassert>
#include <map>

#include "fragment.hpp"

class Schedule {
public:
    void setStopCount(size_t stopCount) {
        maxStopCount_ = stopCount;
    }
    void addTimeLine(size_t fromIx, const TimeLine& tline) {
        assert(tline.size() <= maxStopCount_);

        fragments_[std::make_pair(fromIx, tline.size())].addTimeLine(tline);
    }

    TimeLine getStopTimes(size_t stopIndex) const;

    Time getArriveTime(const Stop& from, Time leave, const Stop& to) const {
        const auto& schedule = schedules_[day];
        auto        timesA = schedule.getStopTimes(stopIndex(from));
        auto        timeLineIx = std::find(timesA.cbegin(), timesA.cend(), leave) - timesA.cbegin();
        return      schedule.getTime(timeLineIx, stopIndex(to));
    }

private:
    using FragmentIndex = std::pair<size_t, size_t>;
    using Fragments = std::map<FragmentIndex, Fragment>;

    Fragments   fragments_;
    size_t      maxStopCount_;
};

#endif // SCHEDULE_HPP
