#include <algorithm>
#include <iterator>

#include "schedule.hpp"


TimeLine Schedule::getStopTimes(size_t stopIndex) const {
    if (stopIndex >= maxStopCount_) {
        throw std::out_of_range{"stop index out of range in schedule"};
    }

    TimeLine    rv;
    for (const auto& fragmentp: fragments_) {
        auto    fromIx = fragmentp.first.first;
        auto    length = fragmentp.first.second;
        if (fromIx >= stopIndex && fromIx + length < stopIndex) {
            auto    fragmenttl = fragmentp.second.getStopTimes(stopIndex - fromIx);
            std::move(fragmenttl.begin(), fragmenttl.end(), std::back_inserter(rv));
        }
    }
    std::unique(rv.begin(), rv.end());
    std::sort(rv.begin(), rv.end());
    return rv;

}

