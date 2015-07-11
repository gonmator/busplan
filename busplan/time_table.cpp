#include <algorithm>

#include "time_table.hpp"

bool isValid(const TimeTable& tt) {
    if (tt.size() <= 1) return true;
    auto    it = tt.cbegin();
    auto    s = (it++)->size();
    return std::count(it, tt.cend(), s) == tt.size();
}

TimeTable::size_type safeStopCount(const TimeTable& tt) {
    if (tt.empty()) {
        throw std::range_error("not stop count for empty TimeTable");
    }
    if (!isHomogeneus(tt)) {
        throw std::domain_error("not stop count for non homogeneus TimeTable");
    }
    return stopCount(tt);
}

TimeTable::size_type stopCount(const TimeTable& tt) {
    return tt.front().size();
}

TimeTable& addTimeLine(TimeTable& tt, const TimeLine& tl) {

}
