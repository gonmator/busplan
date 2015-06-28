#pragma once
#ifndef SCHEDULE_HPP
#define SCHEDULE_HPP

#include <cassert>
#include <map>

#include "fragment.hpp"

class Schedule {
public:
    void setStopCount(size_t stopCount) {
        maxStopCount_ = stopCount;
    }
    void addTimeLine(size_t fromIx, const TimeLine& tline) {
        assert(tline.size() <= maxStopCount_);

        auto    fragmentIx = std::make_pair(fromIx, tline.size());
        auto&   fragment = fragments_[fragmentIx];
        fragment.addTimeLine(tline);
    }

    TimeLine getStopTimes(size_t stopIndex) const;

    Time getArriveTime(size_t fromIx, Time leave, size_t toIx) const;

private:
    using FragmentIndex = std::pair<size_t, size_t>;
    using Fragments = std::map<FragmentIndex, Fragment>;

    static size_t getStopIndex(const FragmentIndex& fix) {
        return fix.first;
    }
    static size_t getStopCount(const FragmentIndex& fix) {
        return fix.second;
    }

    static bool isStopInFragment(const FragmentIndex& fix, size_t stopIx) {
        return getStopIndex(fix) <= stopIx && getStopIndex(fix) + getStopCount(fix) > stopIx;
    }
    Fragments   fragments_;
    size_t      maxStopCount_;
};

#endif // SCHEDULE_HPP
