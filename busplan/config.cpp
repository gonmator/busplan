#include <cassert>
// #include <iostream>
#include <string>

#include "config.hpp"
#include "time_line.hpp"
#include "walking.hpp"


void read(const Utility::IniDoc::Doc&, const std::string&, Line&);
void read(const Utility::IniDoc::Doc&, const std::string&, Route&);
void read(const Utility::IniDoc::Doc&, const std::string&, size_t, DifTimeLines&dtlines);
void read(const Utility::IniDoc::Doc&, const std::string&, size_t, const DifTimeLines&, Schedule&);
void read(const Utility::IniDoc::Doc& cfg, WalkingTimes& wt);

void read(const Utility::IniDoc::Doc &cfg, Lines& lines, StopDescriptions& sds) {
    const auto& ssection = cfg.at("stops");
    for (const auto& sline: ssection) {
        const auto& sdlist = sline.second.items();
        sds.emplace(
            std::piecewise_construct,
            std::forward_as_tuple(sline.first),
            std::forward_as_tuple(sdlist.cbegin(), sdlist.cend()));
        StopDescription sd;
    }

    const auto& lineslist = cfg.at("").at("lines").items();
    for (const auto& lstr: lineslist) {
//        std::cout << "Line: " << lstr << std::endl;

        auto&   line = lines.addLine(lstr);
        read(cfg, lstr, line);
    }

    read(cfg, lines.walkingTimes());
}

void read(const Utility::IniDoc::Doc& cfg, const std::string& sname, Line& line) {
    const auto& routelist = cfg.at(sname).at("routes").items();
    for (const auto& rstr: routelist) {
//        std::cout << "    Route: " << rstr << std::endl;

        auto&   route = line.addRoute(rstr);
        read(cfg, sname + "." + rstr, route);
    }
}

void read(const Utility::IniDoc::Doc& cfg, const std::string& sname, Route& route) {
//    std::cout << "        Stops: " << cfg.at(sname).at("stops").string() << std::endl;
    const auto& stoplist = cfg.at(sname).at("stops").items();
    for (const auto& sstr: stoplist) {
        route.addStop(sstr);
    }

//    std::cout << "        Platforms: " << std::endl;
    const auto& psection = cfg.at(sname + ".platforms");
    for (const auto& pprop: psection) {
        const auto& pstr = pprop.second.string();
//        std::cout << "            " << pprop.first << " -> " << pstr << std::endl;

        route.addPlatform(pprop.first, pstr);
    }

    DifTimeLines    dtimeLines;
    read(cfg, sname + ".times", stoplist.size(), dtimeLines);

//    std::cout << "        Schedule Monday to Friday: " << std::endl;
    read(cfg, sname + ".times.MonToFri", stoplist.size(), dtimeLines, route.monToFri());

//    std::cout << "        Schedule Saturday: " << std::endl;
    read(cfg, sname + ".times.Sat", stoplist.size(), dtimeLines, route.sat());

//    std::cout << "        Schedule Sunday: " << std::endl;
    read(cfg, sname + ".times.Sun", stoplist.size(), dtimeLines, route.sun());
}

void read(
    const Utility::IniDoc::Doc& cfg, const std::string& sname, size_t stopCount, DifTimeLines& dtlines) {
//    std::cout << "        Time lines:" << std::endl;
    if (!cfg.count(sname)) {
        return;
    }
    const auto& section = cfg.at(sname);
    for (const auto& tlprop: section) {
//        std::cout << "            " << tlprop.first << " -> " << tlprop.second.string() << std::endl;

        DifTimeLine&   dtline = dtlines[tlprop.first];
        dtline.assign(stopCount, toDifTime("23:59"));
        size_t      i = 0;
        for (const auto& dtstr: tlprop.second.items()) {
            if (dtstr.empty()) {
                dtline[i] = -dtline[i];
            } else {
                dtline[i] = toDifTime(dtstr);
            }
            ++i;
        }
    }
}

void read(
    const Utility::IniDoc::Doc& cfg,
    const std::string&          sname,
    size_t                      stopCount,
    const DifTimeLines&         dtlines,
    Schedule&                   schedule) {

    if (!cfg.count(sname)) {
        return ;
    }

    schedule.setStopCount(stopCount);
    const auto& section = cfg.at(sname);
    for (const auto& tzprop: section) {
        const auto& tzstr = tzprop.first;
        const auto& tzone = tzprop.second.items();
        auto        dpos = tzstr.find('-');
        Time::rep   ftzone = std::stoul(tzstr.substr(0, dpos));
        auto        ltzone = ftzone + 1;

        if (dpos != tzstr.npos) {
            ltzone = std::stoul(tzstr.substr(dpos + 1)) + 1;
        }
        for (auto i = ftzone; i < ltzone; ++i) {
            for (const auto& tlstr: tzone) {
                assert(dtlines.at(tlstr).size() == stopCount);

                auto    tline = applyTimeZone(dtlines.at(tlstr), i);
                schedule.addTimeLine(tline);

//                std::cout << "            ";
                for (const auto& t: tline) {
                    if (t.time_since_epoch().count() >= 0) {
//                        std::cout << toString(t) << " ";
                    } else {
//                        std::cout << "--:-- ";
                    }
                }
//                std::cout << std::endl;
            }
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


