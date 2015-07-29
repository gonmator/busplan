#include <algorithm>
#include <functional>
#include <boost/graph/dijkstra_shortest_paths.hpp>

#include "bus_network.hpp"
#include "logger.hpp"
#include "time.hpp"

#include <iostream>

namespace {
DifTime adjustDelay;
const DifTime noDelay{std::chrono::minutes{0}};
}

inline DifTime adjust(const RouteId& rida, const RouteId& ridb) {
    if (rida == ridb || rida == walkingRouteId) {
        return DifTime{0};
    }
    return adjustDelay;
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
                const auto& to = step.first;
                const auto& from = step.second;
                /*auto    e =*/ boost::add_edge(
                    stopMap_.at(to),
                    stopMap_.at(from),
                    Section{RouteId{linen, routen}, from, to}, graph_);
//                std::clog << "edge " << e.first << ": " << from << " -> " << to << std::endl;
            }
        }
    }
    for (const auto& walkingTime: lines_.walkingTimes()) {
        const auto& from = walkingTime.first.first;
        const auto& to = walkingTime.first.second;
        boost::add_edge(
            stopMap_.at(to), stopMap_.at(from), Section{walkingRouteId, from, to}, graph_);
        boost::add_edge(
            stopMap_.at(from), stopMap_.at(to), Section{walkingRouteId, to, from}, graph_);
    }
}

BusNetwork::NodeList BusNetwork::planFromArrive(
    Day day, const Stop& from, const Stop& to, Time arrive, Details details, DifTime delay) {

    return planFromArrive(day, from, to, StopTime{{}, arrive}, details, delay);
}

BusNetwork::NodeList BusNetwork::planFromArrive(
    Day day, const Stop& from, const Stop& to, StopTime arriveBy, Details details, DifTime delay) {

    BusNetwork::NodeList    rv;

    using DistanceMap = std::map<VertexDesc, StopTime>;
    using PredecessorMap = std::map<VertexDesc, VertexDesc>;
    using WeightMap = std::map<EdgeDesc, SectionTime>;

    PredecessorMap  p;
    DistanceMap     d;
    WeightMap       w;

    struct Visitor {
        Visitor(const DistanceMap& dis, const WeightMap& wei): log(Logger{}.log("debug")), d(dis), w(wei) {}

        void initialize_vertex(VertexDesc u, const Graph& g) {
            log << "initialize " << g[u] << std::endl;
        }
        void examine_vertex(VertexDesc u, const Graph& g) {
            log << "examine " << g[u] << std::endl;
        }
        void examine_edge(const EdgeDesc& e, const Graph& g) {
            const auto& section = g[e];
            log << "examine " << section.to << " <- " << section.routeid.linen << "." << section.routeid.routen;
            log << " <- " << section.from << std::endl;
        }
        void discover_vertex(VertexDesc u, const Graph& g) {
            log << "discover " << g[u] << std::endl;
        }
        void edge_relaxed(const EdgeDesc& e, const Graph& g) {
            const auto& section = g[e];
            auto    u = boost::source(e, g);
            auto    v = boost::target(e, g);
            log << "relaxed " << section.to << "(" << toString(d.at(u).time) << ") <- ";
            log << section.routeid.linen << "." << section.routeid.routen;
            log << " <- " << section.from << "(" << toString(d.at(v).time) << ")" << std::endl;
        }
        void edge_not_relaxed(const EdgeDesc& e, const Graph& g) {
            const auto& section = g[e];
            auto    u = boost::source(e, g);
            auto    v = boost::target(e, g);
            log << "not relaxed " << section.to << "(" << toString(d.at(u).time) << ") <- ";
            log << section.routeid.linen << "." << section.routeid.routen;
            log << " <- " << section.from << "(" << toString(d.at(v).time) << ")" << std::endl;
        }
        void finish_vertex(VertexDesc u, const Graph& g) {
            log << "finish " << g[u] << std::endl;
        }
    private:
        Log&                log;
        const DistanceMap&  d;
        const WeightMap&    w;
    };


    auto    compare = [](const StopTime& stopta, const StopTime& stoptb) -> bool {
        return stopta.time > stoptb.time /*+ adjust(stopta.routeid, stoptb.routeid)*/;
    };
    auto    combine = [this, day](const StopTime& stopt, const SectionTime& sectiont) -> StopTime {
        Time    arrive = stopt.time - adjust(stopt.routeid, sectiont.routeid);
        auto    leave = this->lines_.getLeaveTime(day, sectiont.routeid, sectiont.from, sectiont.to, arrive);
        return StopTime{sectiont.routeid, leave};
    };

    auto    edger = boost::edges(graph_);
    std::for_each(edger.first, edger.second, [this, &w, day](const EdgeDesc& ed) {
        const auto& section = graph_[ed];
        w[ed] = SectionTime{section.routeid, section.from, section.to};
    });

    Visitor vis(d,w);
    auto    u = stopMap_[to];
    adjustDelay = delay;
    boost::dijkstra_shortest_paths(
        graph_,
        u,
        boost::predecessor_map(boost::associative_property_map<PredecessorMap>(p)).
            distance_map(boost::associative_property_map<DistanceMap>(d)).
            weight_map(boost::associative_property_map<WeightMap>(w)).
            distance_compare(compare).
            distance_combine(combine).
            distance_zero(std::move(arriveBy)).
            distance_inf(StopTime{{}, minusInf}).
            visitor(vis));

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
        const auto& to = section.to;
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

BusNetwork::Table BusNetwork::table(Day day, const Stop& from, const Stop& to, Details details, DifTime delay) {
    Table   rv;
    auto    timeline = lines_.getStopTimes(day, to);
    Time    lastLeave{minusInf};
    Time    lastArrive{plusInf};
    for (const auto& time: timeline) {
        auto    nlist = planFromArrive(day, from, to, time + delay, details, delay);
        if (!nlist.empty()) {
            auto    leave = nlist.front().from.time;
            auto    arrive = nlist.back().to.time;
            if (leave > lastLeave || arrive == lastArrive) {
                rv.push_back(nlist);
                lastLeave = leave;
                lastArrive = arrive;
            }
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


