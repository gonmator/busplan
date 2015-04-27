#pragma once
#ifndef SCHEDULE_HPP
#define SCHEDULE_HPP

#include <cassert>
#include <iterator>
#include <vector>

#include "time.hpp"
#include "time_line.hpp"

class Schedule {
public:
    void setStopCount(size_t stopCount) {
        stopCount_ = stopCount;
    }
    void addTimeLine(const TimeLine& tline) {
        assert(tline.size() == stopCount_);

        timeTable_.insert(timeTable_.end(), tline.cbegin(), tline.cend());
        ++timeLinesCount_;

        assert(timeTable_.size() == stopCount_ * timeLinesCount_);
    }

    TimeLine getStopTimes(size_t stopIndex) const {
        assert(stopIndex < stopCount_);

        TimeLine    rv;
        for (size_t i = 0; i < timeLinesCount_; ++i) {
            rv.push_back(timeTable_[i * stopCount_ + stopIndex]);
        }
        return rv;
    }

private:
    using TimeTable = std::vector<Time>;
    struct TimeTableIterator: std::forward_iterator_tag {
        TimeTableIterator();
    };

    size_t      stopCount_;
    size_t      timeLinesCount_;
    TimeTable   timeTable_;
};

#endif // SCHEDULE_HPP
