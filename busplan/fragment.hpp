#pragma once
#ifndef FRAGMENT_HPP
#define FRAGMENT_HPP

#include <cassert>
#include <iterator>
#include <utility>
#include <vector>

#include "time_table.hpp"


class Fragment {
public:
    Fragment(size_t stopCount): timeTable_{stopCount} {}

    void addTimeLine(const TimeLine& tline) {
        return timeTable_.addTimeLine(tline);
    }
    TimeLine getStopTimes(size_t stopIndex) const {
        return timeTable_.getStopTimes(stopIndex);
    }
    Time getTime(size_t timelineIx, size_t stopIx) const {
        return timeTable_.getTime(timelineIx, stopIx);
    }

    std::pair<Time, bool> findArriveTime(size_t fromIndex, Time leave, size_t toIndex) const;

private:
    TimeTable   timeTable_;
};

#endif // FRAGMENT_HPP
