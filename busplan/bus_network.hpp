#pragma once
#ifndef BUS_NETWORK_HPP
#define BUS_NETWORK_HPP

#include <set>
#include <utility>
#include <vector>
#include <boost/graph/adjacency_list.hpp>

#include "day.hpp"
#include "details.hpp"
#include "lines.hpp"
#include "stop.hpp"

class BusNetwork {
public:
    struct RoutePoint {
        Stop        stop;
        Time        time;
        std::string platform;
    };
    struct Node {
        RoutePoint  from;
        RoutePoint  to;
        RouteId     routeid;
    };
    using NodeList = std::vector<Node>;
    using Table = std::vector<NodeList>;

    BusNetwork(Lines&& lines);

    LineNames getLineNames() const {
        return lines_.getLineNames();
    }
    RouteNames getRouteNames(const LineName& linen) const {
        return lines_.getRouteNames(linen);
    }

    NodeList planFromArrive(
        Day day, const Stop& from, const Stop& to, Time arrive, Details details, DifTime delay);
    Table table(Day day, const Stop& from, const Stop& to, Details details, DifTime delay);

    std::string routeName(const RouteId& routeid) const;

private:
    using StopTime = TimeStep;
    struct Section {
        Section() = default;
        Section(RouteId rid, Stop from, Stop to): routeid{std::move(rid)}, from{std::move(from)}, to{std::move(to)} {
        }

        RouteId routeid;
        Stop    from;
        Stop    to;
    };
    using Graph = boost::adjacency_list<
        boost::multisetS, boost::vecS, boost::directedS, Stop, Section>;
    using VertexDesc = boost::graph_traits<Graph>::vertex_descriptor;
    using EdgeDesc = boost::graph_traits<Graph>::edge_descriptor;

    NodeList planFromArrive(
        Day day, const Stop& from, const Stop& to, TimeStep arriveBy, Details details, DifTime delay);

    void init();
    static NodeList fromStepToTransferList(const NodeList& stepList);
    static NodeList fromTransferToEndList(const NodeList& transferList);
    static NodeList fromStepToEndList(const NodeList& stepList);

    Lines                       lines_;
    Graph                       graph_;
    std::map<Stop, VertexDesc>  stopMap_;
};

#endif // BUS_NETWORK_HPP
