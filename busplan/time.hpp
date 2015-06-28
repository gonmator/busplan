#pragma once
#ifndef TIME_HPP
#define TIME_HPP

#include <chrono>
#include <istream>
#include <ratio>
#include <string>

class Clock {
public:
    using rep = int;
    using period = std::ratio<60>;
    using duration = std::chrono::duration<rep, period>;
    using time_point = std::chrono::time_point<Clock, duration>;
    static const bool is_steady = false;
    static time_point now() {
        using Days = std::chrono::duration<std::chrono::system_clock::rep, std::ratio<86400>>;
        auto    scnow = std::chrono::system_clock::now();
        auto    days = std::chrono::duration_cast<Days>(scnow.time_since_epoch());
        auto    minutes = std::chrono::duration_cast<duration>(scnow.time_since_epoch());
        return  time_point{minutes - days};
    }
};

using Time = Clock::time_point;
using DifTime = Time::duration;

const Time  minusInf{std::chrono::hours{-24}};

inline DifTime toDifTime(std::string str) {
    unsigned long   m = 0;
    auto            colonp = str.find(':');
    if (colonp != str.npos) {
        m = std::stoul(str.substr(0, colonp)) * 60;
        str.erase(0, colonp + 1);
    }
    m += std::stoul(str);
    return std::chrono::minutes{m};
}

inline DifTime toDifTime(size_t m) {
    return std::chrono::minutes(m);
}

inline Time toTime(const std::string& str) {
    return Time{toDifTime(str)};
}

inline std::string toString(Time t) {
    if (t.time_since_epoch() < DifTime{0}) {
       return std::string{"-"} + toString(Time{DifTime{-t.time_since_epoch()}});
    }
    static const char dec[] = "0123456789";
    std::string rv;
    auto        dur = t.time_since_epoch();
    auto        h = std::chrono::duration_cast<std::chrono::hours>(dur);
    auto        m = dur - h;
    h %= 24;

    if (h.count() < 10) {
        rv.append(1, '0');
    } else {
        rv.append(1, dec[h.count() / 10]);
    }
    rv.append(1, dec[h.count() % 10]).append(1, ':');
    if (m.count() < 10) {
        rv.append(1, '0');
    } else {
        rv.append(1, dec[m.count() / 10]);
    }
    rv.append(1, dec[m.count() % 10]);

    return rv;
}

inline std::istream& operator>>(std::istream& is, Time& t) {
    std::string ts;
    is >> ts;
    t = toTime(ts);
    return is;
}

#endif // TIME_LINE_HPP
