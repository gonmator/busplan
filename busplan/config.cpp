#include <algorithm>
#include <cassert>
#include <iostream>
#include <string>

#include "config.hpp"
#include "time_line.hpp"
#include "walking.hpp"


void read(const Utility::IniDoc::Doc&, const std::string&, Line&);
void read(const Utility::IniDoc::Doc&, const std::string&, Route&);
void readPlatforms(const Utility::IniDoc::Doc&, const std::string&, Route&);
void read(const Utility::IniDoc::Doc&, const std::string&, const Stops&, DifTimeLines&dtlines);
void read(const Utility::IniDoc::Doc&, const std::string&, const Stops&, const DifTimeLines&, Schedule&);
void read(const Utility::IniDoc::Doc& cfg, WalkingTimes& wt);

inline std::string strip(std::string str) {
    char    c = ' ';
    auto    p0 = str.find_first_not_of(c);
    auto    p1 = str.find_last_not_of(c);
    if (p1 != str.npos) {
        str.erase(p1 + 1);
    }
    str.erase(0, p0);
    return str;
}

void read(const Utility::IniDoc::Doc &cfg, Lines& lines, StopDescriptions& sds) {
    const auto& ssection = cfg.at("stops");
    for (const auto& sline: ssection) {
        const auto& sdlist = sline.second.items();
        sds.emplace(
            std::piecewise_construct,
            std::forward_as_tuple(sline.first),
            std::forward_as_tuple(sdlist.cbegin(), sdlist.cend()));
    }

    try {
        const auto& lineslist = cfg.at("").at("lines").items();
        for (const auto& lstr: lineslist) {
    //        std::cout << "Line: " << lstr << std::endl;

            auto&   line = lines.addLine(lstr);
            try {
                read(cfg, lstr, line);
            } catch (const std::out_of_range&) {
                std::cerr << "Error in line: " << lstr << std::endl;
                lines.removeLine(lstr);
            }
        }
    } catch (const std::out_of_range&) {
        std::cerr << "Missing 'line' property." << std::endl;
        return ;
    }

    read(cfg, lines.walkingTimes());
}

void read(const Utility::IniDoc::Doc& cfg, const std::string& sname, Line& line) {
    const auto& routelist = cfg.at(sname).at("routes").items();
    for (const auto& rstr: routelist) {
//        std::cout << "    Route: " << rstr << std::endl;

        auto&   route = line.addRoute(rstr);
        try {
            read(cfg, sname + "." + rstr, route);
        } catch (const std::out_of_range&) {
            std::cerr << "Error in route: " << rstr << " (" << sname << ")" << std::endl;
            line.removeRoute(rstr);
        }
    }
}

void read(const Utility::IniDoc::Doc& cfg, const std::string& sname, Route& route) {
    route.setDescription(cfg.at(sname).at("description").string());

//    std::cout << "        Stops: " << cfg.at(sname).at("stops").string() << std::endl;
    const auto& stoplist = cfg.at(sname).at("stops").items();
    for (const auto& sstr: stoplist) {
        route.addStop(strip(sstr));
    }

    readPlatforms(cfg, sname + ".platforms", route);

    DifTimeLines    dtimeLines;
    read(cfg, sname + ".durations", route.stops(), dtimeLines);

    Day day{};
    const auto& timetables = cfg.at(sname).at("timetables").items();
    for (auto day: week) {
        auto&   schedule = route.schedule(day);
        schedule.setStopCount(stoplist.size());
        if (day >= timetables.size()) {
            std::cerr << "Missing day " << day << " (" << sname << ")" << std::endl;
            continue;
        }
        const auto& ttstr = timetables[day];
        if (ttstr.empty()) {
            continue;
        }
        try {
            read(cfg, sname + "." + ttstr, route.stops(), dtimeLines, route.schedule(day));
        } catch (const std::out_of_range&) {
            std::cerr << "Error in day: " << day << "(" << sname << ")" << std::endl;
        }
    }
}


void readPlatforms(const Utility::IniDoc::Doc& cfg, const std::string& sname, Route& route) {
    auto    sectionIt = cfg.find(sname);
    if (sectionIt != cfg.cend()) {
        //    std::cout << "        Platforms: " << std::endl;
        for (const auto& pprop: sectionIt->second) {
            const auto& pstr = pprop.second.string();
        //        std::cout << "            " << pprop.first << " -> " << pstr << std::endl;

            route.addPlatform(pprop.first, pstr);
        }
    }
}

//  read [<line>.<route>.durations]
void read(
    const Utility::IniDoc::Doc& cfg, const std::string& sname, const Stops& stops, DifTimeLines& dtlines) {
//    std::cout << "        Time lines:" << std::endl;
    if (!cfg.count(sname)) {
        return;
    }
    const auto& section = cfg.at(sname);
    for (const auto& tlprop: section) {
//        std::cout << "            " << tlprop.first << " -> " << tlprop.second.string() << std::endl;

        auto&       dtline = dtlines[tlprop.first];
        const auto& dtlist = tlprop.second.items();
        auto        dtit = dtlist.cbegin();
        auto        stopIt = std::find(stops.cbegin(), stops.cend(), strip(*dtit));
        if (stopIt != stops.cend()) {
            dtline.from = strip(*dtit++);
        } else {
            dtline.from = stops.front();
        }
        std::for_each(dtit, dtlist.cend(), [&dtline](const std::string& dtstr) {
            dtline.durations.push_back(toDifTime(strip(dtstr)));
        });
    }
}




//  read [<line>.<route>.<timetable>]
void read(
    const Utility::IniDoc::Doc& cfg,
    const std::string&          sname,
    const Stops&                stops,
    const DifTimeLines&         dtlines,
    Schedule&                   schedule) {

    if (!cfg.count(sname)) {
        return ;
    }

    const auto& section = cfg.at(sname);
    for (const auto& tgprop: section) {
        auto        startTime = toTime(tgprop.first);
        const auto& tgdesc = tgprop.second.items();
        //  todo: manejar excepciones
        const auto& durId = tgdesc.at(0);
        auto        rep = tgdesc.size() > 1 ? tgdesc.at(1).asDecimal<size_t>() : 1;
        auto        cadency = toDifTime(tgdesc.size() > 2 ? tgdesc.at(2).asDecimal<size_t>() : 0);
        const auto& dtline = dtlines.at(durId);
        auto        fromIx = std::find(stops.cbegin(), stops.cend(), dtline.from) - stops.cbegin();
        while (rep--) {
            auto    tline = applyDurations(dtline, startTime);
            schedule.addTimeLine(fromIx, tline);
            startTime += cadency;
        }
    }
}

void read(const Utility::IniDoc::Doc& cfg, WalkingTimes& wt) {
    const auto wsectionIt = cfg.find("walking");
    if (wsectionIt == cfg.cend()) {
        return ;
    }
    for (const auto& wline: wsectionIt->second) {
        auto    ws = Utility::Literal{wline.first}.asList();
        if (ws.size() != 2) {
            throw std::runtime_error("invalid walking entry");
        }
        wt.emplace(
            std::piecewise_construct,
            std::forward_as_tuple(ws[0], ws[1]),
            std::forward_as_tuple(toDifTime(wline.second.string())));
    }
}


