#include <algorithm>
#include <stdexcept>
#include <iterator>

#include <iostream>

#include "fragment.hpp"


std::pair<Time, bool> Fragment::findArriveTime(size_t fromIndex, Time leave, size_t toIndex) const {
    auto   timeLine = timeTable_.lowerBoundTimeLine(fromIndex, leave);
    return timeLine ? std::make_pair((*timeLine)[toIndex - fromIndex], true) : std::make_pair(Time{}, false);
}


