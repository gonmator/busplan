#include <algorithm>
#include <stdexcept>
#include <iterator>

#include <iostream>

#include "fragment.hpp"


std::pair<Time, bool> Fragment::findArriveTime(size_t fromIndex, Time leave, size_t toIndex) const {
    auto   timeLine = timeTable_.notLessThanTimeLine(fromIndex, leave);
    return timeLine ? std::make_pair((*timeLine)[toIndex], true) : std::make_pair(Time{}, false);
}

std::pair<Time, bool> Fragment::findLeaveTime(size_t fromIndex, size_t toIndex, Time arrive) const {
    auto    timeLine = timeTable_.notGreaterThanTimeLine(toIndex, arrive);
    return timeLine ? std::make_pair((*timeLine)[fromIndex], true) : std::make_pair(Time{}, false);
}
