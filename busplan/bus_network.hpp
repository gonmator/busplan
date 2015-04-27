#pragma once
#ifndef BUS_NETWORK_HPP
#define BUS_NETWORK_HPP

#include <set>
#include <utility>
#include <vector>
#include <boost/graph/adjacency_list.hpp>

#include "lines.hpp"
#include "stop.hpp"

class BusNetwork {
public:
    struct Node {
        RouteId     routeid;
        Stop        stop;
        Time        time;
        std::string platform;
    };
    using NodeList = std::vector<Node>;
    using Day = Route::Day;

    BusNetwork(Lines&& lines);

    LineNames getLineNames() const {
        return lines_.getLineNames();
    }
    RouteNames getRouteNames(const LineName& linen) const {
        return lines_.getRouteNames(linen);
    }
    NodeList planFromArrive(Day day, const Stop& from, const Stop& to, Time& arrive);

private:

    struct Section {
        RouteId routeid;
        Stop    from;
        Stop    to;
    };
    struct StopTime {
        RouteId routeid;
        Time    time;
    };
    struct SectionTime {
        SectionTime(RouteId rid, TimeLine tl): routeid{std::move(rid)}, stopTimes{std::move(tl)}, diftime{} {
        }
        SectionTime(RouteId rid, DifTime dt): routeid{std::move(rid)}, stopTimes{}, diftime{std::move(dt)} {
        }
        SectionTime() = default;

        RouteId     routeid;
        TimeLine    stopTimes;
        DifTime     diftime;
    };
    using Graph = boost::adjacency_list<
        boost::multisetS, boost::vecS, boost::directedS, Stop, Section>;
    using VertexDesc = boost::graph_traits<Graph>::vertex_descriptor;
    using EdgeDesc = boost::graph_traits<Graph>::edge_descriptor;

    void init();

    Lines                       lines_;
    Graph                       graph_;
    std::map<Stop, VertexDesc>  stopMap_;
};

#endif // BUS_NETWORK_HPP
