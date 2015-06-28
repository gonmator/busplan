#include <algorithm>

#include "stop.hpp"

std::string checkStop(const StopDescriptions& stopDescriptions, const std::string& stopName) {
    auto    it =
        std::find_if(stopDescriptions.cbegin(), stopDescriptions.cend(), [&stopName](const StopDescription& sd) {

        return
            sd.first == stopName || std::find(sd.second.cbegin(), sd.second.cend(), stopName) != sd.second.cend();
    });
    if (it == stopDescriptions.cend()) {
        return "";
    }
    return it->first;
}

