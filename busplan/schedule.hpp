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

        auto    stopCount = tline.size();
        auto    fragmentIx = std::make_pair(fromIx, stopCount);
        auto    fragmentIt = fragments_.find(fragmentIx);
        if (fragmentIt == fragments_.end()) {
            fragmentIt = fragments_.emplace(fragmentIx, tline.size()).first;
        }
        fragmentIt->second.addTimeLine(tline);
    }

    TimeLine getStopTimes(size_t stopIndex) const;

    Time getArriveTime(size_t fromIx, Time leave, size_t toIx) const;
    Time getLeaveTime(size_t fromIx, size_t toIx, Time arrive) const;

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
