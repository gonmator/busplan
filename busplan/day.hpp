#pragma once
#ifndef DAY_HPP
#define DAY_HPP

#include <cassert>
#include <istream>
#include <ostream>
#include <string>

class Day {
public:
    Day(size_t d = 0): day_{d % 7} {
        assert(d < 7);
    }
    Day(const std::string& str);
    operator size_t() const {
        return day_;
    }

    Day(const Day&) = default;
    Day& operator= (const Day&) = default;

private:
    size_t  day_;
};

inline std::istream& operator>>(std::istream& is, Day& d) {
    std::string dstr;
    is >> dstr;
    d = Day(dstr);
    return is;
}

inline std::ostream& operator<<(std::ostream& os, Day d) {
    return os << size_t{d};
}

const Day                   sunday{0};
const Day                   monday{1};
const Day                   tuesday{2};
const Day                   wednesday{3};
const Day                   thursday{4};
const Day                   friday{5};
const Day                   saturday{6};
const std::array<Day, 7>    week{sunday, monday, tuesday, wednesday, thursday, friday, saturday};
#endif // DATE_HPP
