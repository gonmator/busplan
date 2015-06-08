#include <algorithm>
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

std::pair<Time, bool> Fragment::findArriveTime(size_t fromIndex, Time leave, size_t toIndex) const {
    auto    leaves = getStopTimes(fromIndex);
    auto    leaveIt = std::find(leaves.cbegin(), leaves.cend(), leave);
    if (leaveIt == leaves.cend()) {
        return std::make_pair(leave, false);
    }
    return std::make_pair(*(leaveIt + toIndex - fromIndex), true);
}
