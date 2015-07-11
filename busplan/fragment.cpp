#include <algorithm>
#include <stdexcept>
#include <iterator>

#include <iostream>

#include "fragment.hpp"

using TimeTable = std::vector<Time>;


void Fragment::addTimeLine(const TimeLine& tline) {
    assert(timeLinesCount_ == 0 || tline.size() == stopCount());

    timeTable_.insert(
        std::lower_bound(begin().timeTableIterator(), end().timeTableIterator(), tline.front()),
        tline.cbegin(),
        tline.cend());
    ++timeLinesCount_;

    assert(timeTable_.size() == stopCount() * timeLinesCount_);
}

TimeLine Fragment::getStopTimes(size_t stopIndex) const {
    if (stopIndex >= stopCount()) {
        throw std::out_of_range{"stop index out of range in schedule"};
    }

    TimeLine    rv;
    for (size_t i = 0; i < timeLinesCount_; ++i) {
        rv.push_back(timeTable_[i * stopCount() + stopIndex]);
    }
    return rv;
}

std::pair<Time, bool> Fragment::findArriveTime(size_t fromIndex, Time leave, size_t toIndex) const {
    if (fromIndex >= stopCount()) {
        throw std::out_of_range{"stop index out of range in fragment"};
    }

    auto    fromTimes = getStopTimes(fromIndex);
    if (!std::is_sorted(fromTimes.cbegin(), fromTimes.cend())) {
        auto    until = std::is_sorted_until(fromTimes.cbegin(), fromTimes.cend()) - fromTimes.cbegin();
        std::clog << until << std::endl;
    }
    auto    fromTimeIt = std::lower_bound(fromTimes.cbegin(), fromTimes.cend(), leave);
    size_t  timeLineIx = fromTimeIt - fromTimes.cbegin();
    return std::make_pair(getTime(timeLineIx, toIndex), timeLineIx < timeLinesCount_);

//    for (size_t timeLineIx = 0; timeLineIx < timeLinesCount_; ++timeLineIx) {
//        if (leave == getTime(timeLineIx, fromIndex)) {
//            return std::make_pair(getTime(timeLineIx, toIndex), true);
//        }
//    }
//    return std::make_pair(leave, false);
}


