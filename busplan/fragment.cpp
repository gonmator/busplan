#include <stdexcept>

#include "fragment.hpp"

TimeLine Fragment::getStopTimes(size_t stopIndex) const {
    if (stopIndex >= stopCount_) {
        throw std::out_of_range{"stop index out of range in schedule"};
    }

    TimeLine    rv;
    for (size_t i = 0; i < timeLinesCount_; ++i) {
        rv.push_back(timeTable_[i * stopCount_ + stopIndex]);
    }
    return rv;
}
