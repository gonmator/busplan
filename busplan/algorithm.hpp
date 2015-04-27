#pragma once
#ifndef ALGORITHM_HPP
#define ALGORITHM_HPP

#include <iterator>
#include <utility>

template <
    typename InputIt,
    typename OutputIt,
    typename Value = typename std::iterator_traits<InputIt>::value_type>
void setPairs(InputIt first, InputIt last, OutputIt outFirst) {
    if (std::distance(first, last) > 1) {
        auto    prevp = *first++;
        std::for_each(first, last, [&prevp, &outFirst](const Value& p) {
            *outFirst++ = std::make_pair(prevp, p);
            prevp = p;
        });
    }
}

template <typename Cont>
std::vector<typename Cont::key_type>
getKeyVector(const Cont& cont) {
    std::vector<typename Cont::key_type>    rv;
    for (const auto& p: cont) {
        rv.push_back(p.first);
    }
    return rv;
}

#endif // ALGORITHM_HPP
