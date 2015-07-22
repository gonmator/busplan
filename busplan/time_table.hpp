#pragma once
#ifndef TIME_TABLE_HPP
#define TIME_TABLE_HPP

#include <cassert>
#include <vector>

#include "time_line.hpp"

using StopTimes = std::vector<Time>;

class TimeTable {
public:
    TimeTable(size_t stopCount): timeLines_{}, stopCount_{stopCount} {}
    TimeTable() = delete;

    size_t stopCount() const {
        return stopCount_;
    }
    size_t timeLineCount() const {
        return timeLines_.size();
    }

    void addTimeLine(TimeLine timeLine);

    const TimeLine& getTimeLine(size_t timeLineIndex) const {
        assert(timeLineIndex < timeLines_.size());

        return timeLines_[timeLineIndex];
    }
    StopTimes getStopTimes(size_t stopIndex) const;

    Time getTime(size_t timeLineIndex, size_t stopIndex) const {
        assert(timeLineIndex < timeLines_.size());
        assert(stopIndex < stopCount_);

        return timeLines_[timeLineIndex][stopIndex];
    }

    const TimeLine* notLessThanTimeLine(size_t stopIndex, Time time) const;
    const TimeLine* notGreaterThanTimeLine(size_t stopIndex, Time time) const;
private:
    std::vector<TimeLine>   timeLines_;
    size_t                  stopCount_;
};

#endif // TIME_TABLE_HPP
