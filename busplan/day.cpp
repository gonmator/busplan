#include <chrono>
#include <ctime>
#include <map>
// #include <regex>

#include "day.hpp"

namespace {
    const std::map<std::string, size_t>   knownDays{
        {"sunday", 0}, {"monday", 1}, {"tuesday", 2}, {"wednesday", 3}, {"thursday", 4}, {"friday", 5}, {"saturday", 6},
        {"Sunday", 0}, {"Monday", 1}, {"Tuesday", 2}, {"Wednesday", 3}, {"Thursday", 4}, {"Friday", 5}, {"Saturday", 6}
    };

//    const std::regex    iso_date{"[0-9]{1,2}"};
//    const std::regex    inv_date{R"((\d?\d\.\d?\d(\.(\d\d)?\d\d)))"};
}

Day::Day(const std::string &str) {
    auto    itd = knownDays.find(str);
    if (itd != knownDays.cend()) {
        day_ = itd->second;
        return ;
    }

    auto    useday = std::chrono::system_clock::now();
    if (str == "tomorrow" || str == "Tomorrow") {
        useday += std::chrono::hours{24};
    }

    auto    t = std::chrono::system_clock::to_time_t(useday);
    day_ = localtime(&t)->tm_wday;
}
