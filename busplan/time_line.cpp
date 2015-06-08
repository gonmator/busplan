#include "time_line.hpp"

TimeLine applyDurations(const DifTimeLine& dtline, Time start) {
    TimeLine    rv{start};
    for (auto duration: dtline.durations) {
        start += duration;
        rv.push_back(start);
    }
    return rv;
}

