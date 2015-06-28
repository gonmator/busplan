#pragma once
#ifndef FRAGMENT_HPP
#define FRAGMENT_HPP

#include <cassert>
#include <iterator>
#include <utility>
#include <vector>

#include "time.hpp"
#include "time_line.hpp"

class Fragment {
public:
    void addTimeLine(const TimeLine& tline);
    TimeLine getStopTimes(size_t stopIndex) const;

    Time getTime(size_t timelineIx, size_t stopIx) const {
        assert(timelineIx < timeLinesCount_);
        assert(stopIx < stopCount());
        assert(timeTable_.size() == stopCount() * timeLinesCount_);

        return timeTable_[timelineIx * stopCount() + stopIx];
    }

    std::pair<Time, bool> findArriveTime(size_t fromIndex, Time leave, size_t toIndex) const;

private:
    using TimeTable = std::vector<Time>;
    struct TimeTableIterator: std::forward_iterator_tag {
        TimeTableIterator();
    };

    size_t stopCount() const {
        return timeTable_.size() / timeLinesCount_;
    }

    size_t      timeLinesCount_;
    TimeTable   timeTable_;
};

#endif // FRAGMENT_HPP
