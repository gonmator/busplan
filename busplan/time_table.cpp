#include <algorithm>
#include <cassert>
#include <iterator>

#include <boost/iterator/iterator_facade.hpp>

#include "time_table.hpp"

class ConstTimeTableIterator: public boost::iterator_facade<ConstTimeTableIterator, const Time, boost::random_access_traversal_tag> {
public:
    using ConstTimeLineIterator = std::vector<TimeLine>::const_iterator;

    ConstTimeTableIterator(): stopIndex_{}, timeLineIt_{} {}
    ConstTimeTableIterator(size_t stopIndex, ConstTimeLineIterator timeLineIt): stopIndex_{stopIndex}, timeLineIt_{timeLineIt} {}
    ConstTimeTableIterator(const ConstTimeTableIterator&) = default;

    ConstTimeLineIterator timeLineIterator() const {
        return timeLineIt_;
    }
private:
    friend class boost::iterator_core_access;

    const Time& dereference() const {
        return (*timeLineIt_)[stopIndex_];
    }
    bool equal(const ConstTimeTableIterator& other) const {
        return timeLineIt_ == other.timeLineIt_;
    }
    void increment() {
        ++timeLineIt_;
    }
    void decrement() {
        --timeLineIt_;
    }
    void advance(size_t n) {
        timeLineIt_ += n;
    }
    size_t distance_to(const ConstTimeTableIterator& other) const {
        return other.timeLineIt_ - timeLineIt_;
    }

    size_t                  stopIndex_;
    ConstTimeLineIterator   timeLineIt_;
};


void TimeTable::addTimeLine(const TimeLine& timeLine) {
    assert(timeLine.size() == stopCount_);
    if (timeLine.size() != stopCount_) {
        throw std::out_of_range("TimeTable::addTimeLine: invalid time lines size");
    }
    timeLines_.insert(std::lower_bound(timeLines_.cbegin(), timeLines_.cend(), timeLine), timeLine);
}


StopTimes TimeTable::getStopTimes(size_t stopIndex) const {
    assert(stopIndex < stopCount_);

    TimeLine    stopTimes;
    stopTimes.reserve(stopCount_);
    std::for_each(timeLines_.cbegin(), timeLines_.cend(), [&stopTimes, stopIndex](const TimeLine& timeLine) {
        stopTimes.push_back(timeLine[stopIndex]);
    });

    return stopTimes;
}

const TimeLine* TimeTable::lowerBoundTimeLine(size_t stopIndex, Time time) const {
    ConstTimeTableIterator  beg{stopIndex, timeLines_.cbegin()};
    ConstTimeTableIterator  end{stopIndex, timeLines_.cend()};
    auto                    timeTableIt = std::lower_bound(beg, end, time);
    return timeTableIt == end ? nullptr : &*timeTableIt.timeLineIterator();
}
