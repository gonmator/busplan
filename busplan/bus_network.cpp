#include <algorithm>
#include <functional>
#include <boost/graph/dijkstra_shortest_paths.hpp>

#include "bus_network.hpp"
#include "time.hpp"

#include <iostream>

inline DifTime adjust(const RouteId& rida, const RouteId& ridb) {
    if (rida == ridb || rida == walkingRouteId) {
        return DifTime{0};
    }
    return std::chrono::minutes{5};
}

BusNetwork::BusNetwork(Lines&& lines): lines_{std::move(lines)}, graph_{}, stopMap_{} {
    auto    stops = lines_.getStopSet();
    for(const auto& stop: stops) {
        stopMap_[stop] = boost::add_vertex(stop, graph_);
//        std::clog << "vertex " << stopMap_[stop] << ": " << stop << std::endl;
    }
    auto    stepsLines = lines_.getBackwardStepsLines();
    for (const auto& stepsByLine: stepsLines) {
        const auto& linen = stepsByLine.first;
        for (const auto& stepsByRoute: stepsByLine.second) {
            const auto& routen = stepsByRoute.first;
            for (const auto& step: stepsByRoute.second) {
                const auto& from = step.first;
                const auto& to = step.second;
                /*auto    e =*/ boost::add_edge(
                    stopMap_.at(from),
                    stopMap_.at(to),
                    Section{RouteId{linen, routen}, from, to}, graph_);
//                std::clog << "edge " << e.first << ": " << from << " -> " << to << std::endl;
            }
        }
    }
    for (const auto& walkingTime: lines_.walkingTimes()) {
        const auto& from = walkingTime.first.first;
        const auto& to = walkingTime.first.second;
        boost::add_edge(
            stopMap_.at(from), stopMap_.at(to), Section{walkingRouteId, from, to}, graph_);
        boost::add_edge(
            stopMap_.at(to), stopMap_.at(from), Section{walkingRouteId, to, from}, graph_);
    }
}

BusNetwork::NodeList BusNetwork::planFromArrive(
    Day day, const Stop& from, const Stop& to, Time arrive, Details details) {

    BusNetwork::NodeList    rv;

    using DistanceMap = std::map<VertexDesc, StopTime>;
    using PredecessorMap = std::map<VertexDesc, VertexDesc>;
    using WeightMap = std::map<EdgeDesc, SectionTime>;

    PredecessorMap  p;
    DistanceMap     d;
    WeightMap       w;

    auto    compare = [](const StopTime& stopta, const StopTime& stoptb) {
//        std::clog << "\t" << stopta.routeid.linen << "." << stopta.routeid.routen << "\t";
//        std::clog << toString(stopta.time) << "\t";
//        std::clog << "\t" << stoptb.routeid.linen << "." << stoptb.routeid.routen << "\t";
//        std::clog << toString(stoptb.time) << "\t";
//        std::clog << (stopta.time > stoptb.time + adjust(stopta.routeid, stoptb.routeid));
//        std::clog << std::endl;

        return stopta.time > stoptb.time + adjust(stopta.routeid, stoptb.routeid);
    };
    auto    combine = [](const StopTime& stopt, const SectionTime& sectiont) {
        Time    toTime = stopt.time - adjust(stopt.routeid, sectiont.routeid);
        Time    fromTime = minusInf;
        if (sectiont.routeid == walkingRouteId) {
            fromTime = toTime - sectiont.diftime;
        } else {
            auto    fromIt = std::lower_bound(
                sectiont.stopTimes.crbegin(),
                sectiont.stopTimes.crend(),
                toTime,
                std::greater<Time>{});
            if (fromIt != sectiont.stopTimes.crend()) {
                fromTime = *fromIt;
            }
        }
//        std::clog << stopt.routeid.linen << "." << stopt.routeid.routen << "\t";
//        std::clog << toString(stopt.time) << "\t";
//        std::clog << sectiont.routeid.linen << "." << sectiont.routeid.routen << "\t";
//        std::clog << toString(fromTime) << std::endl;

        return StopTime{sectiont.routeid, fromTime};
    };

    auto    edger = boost::edges(graph_);
    std::for_each(edger.first, edger.second, [this, &w, day](const EdgeDesc& ed) {
        const auto& section = graph_[ed];
        if (section.routeid == walkingRouteId) {
            w[ed] = SectionTime{
                section.routeid,
                lines_.walkingTimes().at(WalkingStep{section.from, section.to})};
        } else {
            w[ed] = SectionTime{
                section.routeid,
                lines_.getStopTimes(day, section.routeid, section.to)};
        }
//        std::clog << "weight " << ed << ": ";
//        std::clog << toString(*w[ed].stopTimes.cbegin());
//        std::clog << " ~ " << toString(*(w[ed].stopTimes.cend() - 1));
//        std::clog << std::endl;
    });

    auto    u = stopMap_[to];
    boost::dijkstra_shortest_paths(
        graph_,
        u,
        boost::predecessor_map(boost::associative_property_map<PredecessorMap>(p)).
            distance_map(boost::associative_property_map<DistanceMap>(d)).
            weight_map(boost::associative_property_map<WeightMap>(w)).
            distance_compare(compare).
            distance_combine(combine).
            distance_zero(StopTime{{}, arrive}).
            distance_inf(StopTime{{}, minusInf}));

    auto    edge_list = [this](VertexDesc u, VertexDesc v) {
        std::vector<EdgeDesc>   rv;
        auto                    er = boost::out_edges(u, graph_);
        auto&                   g = graph_;
        std::for_each(er.first, er.second, [&rv, v, &g](EdgeDesc e) {
            if (boost::target(e, g) == v) {
                rv.push_back(e);
            }
        });
        return rv;
    };

    auto    v = stopMap_[from];
    auto    dv = d.at(v);
    Stop    stop = from;
    Time    time = dv.time;
    auto    pred = p[v];
    while (pred != v && v != u) {
        auto    dpred = d.at(pred);
//        auto    e = boost::edge(pred, v, graph_);
        auto    el = edge_list(pred, v);
        auto    eit =
            std::min_element(el.cbegin(), el.cend(), [pred, &dpred, &w, &combine, &compare](
                EdgeDesc e1, EdgeDesc e2) {

            return compare(combine(dpred, w[e1]), combine(dpred, w[e2]));
        });
        auto        section = graph_[*eit];
        const auto& to = section.from;
        const auto& routeid = section.routeid;
        rv.push_back(
            Node{
                {stop, time, lines_.getPlatform(routeid, stop)},
                {to, lines_.getArriveTime(day, routeid, stop, time, to), lines_.getPlatform(routeid, to)},
                routeid});
        stop = graph_[pred];
        time = dpred.time;
        v = pred;
        pred = p[v];
    }
//    rv.push_back(Node{RouteId{}, stop, time, {}});

    if (details == Details::transfers) {
        return fromStepToTransferList(rv);
    }
    if (details == Details::ends) {
        return fromStepToEndList(rv);
    }
    return rv;
}

BusNetwork::Table BusNetwork::table(Day day, const Stop& from, const Stop& to, Details details) {
    Table   rv;
    auto    timeline = lines_.getStopTimes(day, to);
    for (const auto& time: timeline) {
        auto    nlist = planFromArrive(day, from, to, time, details);
        if (!nlist.empty()) {
            rv.push_back(nlist);
        }
    }

    std::stable_sort(rv.begin(), rv.end(), [](const NodeList& nl1, const NodeList& nl2) {
        assert(!nl1.empty());
        assert(!nl2.empty());
        assert(nl1.front().from.stop == nl2.front().from.stop);
        assert(nl1.back().to.stop == nl2.back().to.stop);

        return
            nl1.back().to.time < nl2.back().to.time ||
                (nl1.back().to.time == nl2.back().to.time && nl1.front().from.time > nl2.front().from.time);
    });

    rv.erase(std::unique(rv.begin(), rv.end(), [](const NodeList& nl1, const NodeList& nl2) {
        assert(nl1.back().to.stop == nl2.back().to.stop);

        return nl1.back().to.time == nl2.back().to.time || nl1.front().from.time == nl2.front().from.time;
    }), rv.end());
    return rv;
}

std::string BusNetwork::routeName(const RouteId& routeid) const {
    return lines_.getRouteDescription(routeid);
}

BusNetwork::NodeList BusNetwork::fromStepToTransferList(const BusNetwork::NodeList& stepList) {
    if (stepList.empty()) {
        return NodeList{};
    }

    NodeList    rv;
    RouteId     routeid = stepList.front().routeid;
    RoutePoint  from = stepList.front().from;
    RoutePoint  to;
    for (const auto& node: stepList) {
        if (node.routeid != routeid) {
            rv.push_back(Node{from, to, routeid});
            from = node.from;
        }
        routeid = node.routeid;
        to = node.to;
    }
    rv.push_back(Node{from, to, routeid});
    return rv;
}

BusNetwork::NodeList BusNetwork::fromTransferToEndList(const BusNetwork::NodeList& transferList) {
    if (transferList.empty()) {
        return NodeList{};
    }

    return NodeList{Node{transferList.front().from, transferList.back().to, RouteId{}}};
}

BusNetwork::NodeList BusNetwork::fromStepToEndList(const BusNetwork::NodeList& stepList) {
    return fromTransferToEndList(stepList);
}


