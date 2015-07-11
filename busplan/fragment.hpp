#pragma once
#ifndef FRAGMENT_HPP
#define FRAGMENT_HPP

#include <cassert>
#include <iterator>
#include <utility>
#include <vector>

#include "time.hpp"
#include "time_table.hpp"


struct TimeLineIterator: std::iterator<std::random_access_iterator_tag, Time>  {
    using TimeTableIterator = std::vector<Time>::iterator;

    TimeLineIterator() = default;
    TimeLineIterator(TimeTableIterator it, size_t stopCount): it_{it}, stopCount_{stopCount} {
    }
    TimeLineIterator(const TimeLineIterator&) = default;
    TimeLineIterator& operator=(const TimeLineIterator&) = default;
    ~TimeLineIterator() = default;

    reference operator*() {
        return *it_;
    }
    bool operator==(const TimeLineIterator& rhs) const {
        return rhs.it_ == it_ && rhs.stopCount_ == stopCount_;
    }
    bool operator!=(const TimeLineIterator& rhs) const {
        return !operator==(rhs);
    }
    pointer operator->() {
        return &*it_;
    }
    TimeLineIterator& operator++() {
        it_ += stopCount_;
        return *this;
    }
    value_type operator++(int) {
        auto    rv = *it_;
        it_ += stopCount_;
        return rv;
    }
    TimeLineIterator& operator--() {
        it_ -= stopCount_;
        return *this;
    }
    value_type operator--(int) {
        auto    rv = *it_;
        it_ -= stopCount_;
        return rv;
    }
    TimeLineIterator& operator+=(difference_type n) {
        it_ += stopCount_ * n;
        return *this;
    }
    TimeLineIterator& operator-=(difference_type n) {
        it_ -= stopCount_ * n;
        return *this;
    }
    TimeLineIterator::difference_type operator-(const TimeLineIterator& a) const {
        assert(a.stopCount_ == stopCount_);
        assert((it_ - a.it_) % stopCount_ == 0);
        return (it_ - a.it_) / stopCount_;
    }
    reference operator[](difference_type n) {
        return *(it_ + n * stopCount_);
    }

    TimeTableIterator timeTableIterator() {
        return it_;
    }

private:
    TimeTableIterator   it_;
    size_t              stopCount_;
};

inline TimeLineIterator operator+(TimeLineIterator a, TimeLineIterator::difference_type n) {
    return a += n;
}
inline TimeLineIterator operator+(TimeLineIterator::difference_type n, TimeLineIterator a) {
    return a += n;
}
inline TimeLineIterator operator-(TimeLineIterator a, TimeLineIterator::difference_type n) {
    return a -= n;
}
inline bool operator<(const TimeLineIterator& a, const TimeLineIterator& b) {
    return b - a > 0;
}
inline bool operator>(const TimeLineIterator& a, const TimeLineIterator& b) {
    return b < a;
}
inline bool operator<=(const TimeLineIterator& a, const TimeLineIterator& b) {
    return !(a > b);
}
inline bool operator>=(const TimeLineIterator& a, const TimeLineIterator& b) {
    return !(a < b);
}

class Fragment {
public:
    void addTimeLine(const TimeLine& tline);
    TimeLine getStopTimes(size_t stopIndex) const;

    Time getTime(size_t timelineIx, size_t stopIx) const {
        assert(timelineIx < timeLinesCount_);
        assert(stopIx < stopCount());
        assert(timeTable_.size() == stopCount() * timeLinesCount_);

        return timeTable_[timelineIx * stopCount() + stopIx];
    }

    std::pair<Time, bool> findArriveTime(size_t fromIndex, Time leave, size_t toIndex) const;

private:
    size_t stopCount() const {
        return timeTable_.size() / timeLinesCount_;
    }
    TimeLineIterator begin() {
        return TimeLineIterator{timeTable_.begin(), timeLinesCount_ ? stopCount() : 0};
    }
    TimeLineIterator end() {
        return TimeLineIterator{timeTable_.end() + timeLinesCount_, timeLinesCount_ ? stopCount() : 0};
    }

    size_t      timeLinesCount_;
    TimeTable   timeTable_;
};

#endif // FRAGMENT_HPP
