#include <algorithm>
#include <functional>
#include <iterator>

#include <boost/graph/dijkstra_shortest_paths.hpp>

#include "bus_network.hpp"
#include "logger.hpp"
#include "shortest_path.hpp"
#include "time.hpp"

#include <iostream>

namespace {

DifTime adjustDelay;
const DifTime noDelay{std::chrono::minutes{0}};

inline DifTime adjust(const RouteId& rida, const RouteId& ridb) {
    if (rida == ridb || rida == walkingRouteId) {
        return DifTime{0};
    }
    return adjustDelay;
}


class GraphDistance {
public:
    using Distances = std::map<RouteId, TimeForRoute>;
    using Distance = Distances::value_type;

    GraphDistance() = default;
    GraphDistance(const GraphDistance& gd) = default;
    GraphDistance(GraphDistance&& gd) = default;
    explicit GraphDistance(const TimeForRoute& timeForRoute): distances_{} {
        distances_[timeForRoute.routeid] = TimeForRoute{timeForRoute.time};
    }

    GraphDistance& operator=(const GraphDistance& gd) {
        assign(gd);
        return *this;
    }
    GraphDistance& operator=(GraphDistance&& gd) {
        assign(std::move(gd));
        return *this;
    }

    void assign(const GraphDistance& gd) {
        for (const auto& vt: gd.distances_) {
            distances_[vt.first] = vt.second;
        }
    }
    void assign(GraphDistance&& gd) {
        for (const auto& vt: gd.distances_) {
            distances_[vt.first] = std::move(vt.second);
        }
    }

    const TimeForRoute& at(const RouteId& routeid) const {
        return distances_.at(routeid);
    }
    TimeForRoute& operator[](const RouteId& routeid) {
        return distances_[routeid];
    }

    Distances::const_iterator cbegin() const {
        return distances_.cbegin();
    }
    Distances::const_iterator begin() const {
        return cbegin();
    }
    Distances::iterator begin() {
        return distances_.begin();
    }
    Distances::const_iterator cend() const {
        return distances_.cend();
    }
    Distances::const_iterator end() const {
        return cend();
    }
    Distances::iterator end() {
        return distances_.end();
    }

    bool empty() const {
        return distances_.empty();
    }
    size_t size() const {
        return distances_.size();
    }

    Distances::const_iterator find(const RouteId& routeid) const {
        return distances_.find(routeid);
    }
    Distances::iterator find(const RouteId& routeid) {
        return distances_.find(routeid);
    }

private:
    Distances   distances_;
};

template <typename TCompare>
bool compare(const GraphDistance& gd1, const GraphDistance& gd2, TCompare tcomp) {
    for (const auto& d1: gd1) {
        auto    it2 = gd2.find(d1.first);
        if (it2 == gd2.cend()) {
            return true;
        }
        if (tcomp(d1.second.time, it2->second.time)) {
            return true;
        }
    }
    return false;
}

template <typename TCompare, typename TGet, typename TAdjust>
GraphDistance combine(
    const GraphDistance& gd, const BusNetwork::Section& section, TCompare tcomp, TGet tget, TAdjust tadjust, Time infinite) {

    GraphDistance   rv;
    for (const auto& d: gd) {
        auto    arrive = d.second.time - tadjust(d.first, section.routeid);
        auto    leave = tget(section.routeid, section.from, section.to, arrive);
        if (leave == infinite) {
            continue;
        }
        auto    rit = rv.find(section.routeid);
        if (rit == rv.cend() || tcomp(leave, rit->second.time)) {
            rv[section.routeid] = TimeForRoute{d.first, leave};
        }
    }
    return rv;
}

inline GraphDistance& assign(GraphDistance& gdl, const GraphDistance& gdr) {
    gdl.assign(gdr);
    return gdl;
}

std::ostream& operator<<(std::ostream& os, const GraphDistance::Distance& d) {
    os << d.first << " (time: " << toString(d.second.time) << ", through: " << d.second.routeid << ")";
    return os;
}

std::ostream& operator<<(std::ostream& os, const GraphDistance& gd) {
    if (gd.empty()) {
        return os;
    }
    auto    it = gd.cbegin();
    os << *it++;
    std::for_each(it, gd.cend(), [&os](const GraphDistance::Distance& d) {
        os << ", " << d;
    });
    return os;
}

inline bool arrive_compare(Time t1, Time t2) {
    return t1 > t2;
}

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

    return planFromArrive(day, from, to, TimeForRoute{arrive}, details, delay);
//    auto                    timesByRoute = lines_.getBoundArriveTimesByRoute(day, to, arrive);
//    Time                    bestLeave{minusInf};
//    BusNetwork::NodeList    rv;
//    for (const auto& arriveBy: timesByRoute) {
//        auto    nlist = planFromArrive(day, from, to, arriveBy, details, delay);
//        if (!nlist.empty()) {
//            auto    leave = nlist.front().from.time;
//            if (leave > bestLeave || (leave == bestLeave && nlist.size()) < rv.size()) {
//                rv = nlist;
//                bestLeave = leave;
//            }
//        }
//    }

//    return rv;
}

BusNetwork::NodeList BusNetwork::planFromArrive(
    Day day, const Stop& from, const Stop& to, TimeForRoute arriveBy, Details details, DifTime delay) {

    BusNetwork::NodeList    rv;

    using DistanceMap = std::map<VertexDesc, GraphDistance>;
    using PredecessorMap = std::map<VertexDesc, VertexDesc>;
    using WeightMap = std::map<EdgeDesc, Section>;

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
            log << "examine " << section.to << " <- " << section.routeid << " <- " << section.from << std::endl;
        }
        void discover_vertex(VertexDesc u, const Graph& g) {
            log << "discover " << g[u] << std::endl;
        }
        void edge_relaxed(const EdgeDesc& e, const Graph& g) {
            const auto& section = g[e];
            auto    u = boost::source(e, g);
            auto    v = boost::target(e, g);
            log << "relaxed " << section.to << "[" << d.at(u) <<"] <- " << section.routeid;
            log << " <- " << section.from << "[" << d.at(v) << "]" << std::endl;
        }
        void edge_not_relaxed(const EdgeDesc& e, const Graph& g) {
            const auto& section = g[e];
            auto    u = boost::source(e, g);
            auto    v = boost::target(e, g);
            log << "not relaxed " << section.to << "[" << d.at(u) <<"] <- " << section.routeid;
            log << " <- " << section.from << "[" << d.at(v) << "]" << std::endl;
        }
        void finish_vertex(VertexDesc u, const Graph& g) {
            log << "finish " << g[u] << std::endl;
        }
    private:
        Log&                log;
        const DistanceMap&  d;
        const WeightMap&    w;
    };


    auto    compare = [](const GraphDistance& gda, const GraphDistance& gdb) -> bool {
        return ::compare(gda, gdb, arrive_compare);
    };
    auto    combine = [this, day](const GraphDistance& gd, const Section& section) -> GraphDistance {
        using namespace std::placeholders;

        auto    tget = std::bind(&Lines::getLeaveTime, &this->lines_, day, _1, _2, _3, _4);
        return ::combine(gd, section, arrive_compare, tget, adjust, minusInf);
    };

    auto    edger = boost::edges(graph_);
    std::for_each(edger.first, edger.second, [this, &w, day](const EdgeDesc& ed) {
        const auto& section = graph_[ed];
        w[ed] = Section{section.routeid, section.from, section.to};
    });

    Visitor vis(d,w);
    auto    u = stopMap_[to];
    adjustDelay = delay;
//    boost::dijkstra_shortest_paths(
//        graph_,
//        u,
//        boost::predecessor_map(boost::associative_property_map<PredecessorMap>(p)).
//            distance_map(boost::associative_property_map<DistanceMap>(d)).
//            weight_map(boost::associative_property_map<WeightMap>(w)).
//            distance_compare(compare).
//            distance_combine(combine).
//            distance_zero(GraphDistance{arriveBy}).
//            distance_inf(GraphDistance{}).
//            visitor(vis));

    mod_dijkstra_shortest_path(
        graph_, u, w, d, compare, combine, assign, GraphDistance{arriveBy}, GraphDistance{}, vis);
//    auto    edge_list = [this](VertexDesc u, VertexDesc v) {
//        std::vector<EdgeDesc>   rv;
//        auto                    er = boost::out_edges(u, graph_);
//        auto&                   g = graph_;
//        std::for_each(er.first, er.second, [&rv, v, &g](EdgeDesc e) {
//            if (boost::target(e, g) == v) {
//                rv.push_back(e);
//            }
//        });
//        return rv;
//    };


    Stop        stop = from;
    const auto& dv = d.at(stopMap_[stop]);
    auto        it = std::min_element(
        dv.cbegin(), dv.cend(), [](const GraphDistance::Distance& d1, const GraphDistance::Distance& d2) {

        return ::arrive_compare(d1.second.time, d2.second.time);
    });
    if (it == dv.cend()) {
        return rv;
    }
    auto    routeid = it->first;
    while (stop != to) {
        const auto& dv = d.at(stopMap_[stop]).at(routeid);
        auto        time = dv.time;
        const auto& platform = lines_.getPlatform(routeid, stop);
        auto        nextStop = lines_.getNextStop(routeid, stop);
        auto        arrive = lines_.getArriveTime(day, routeid, stop, time, nextStop);
        const auto& nextPlatform = lines_.getPlatform(routeid, nextStop);

        rv.push_back(Node{{stop, time, platform}, {nextStop, arrive, nextPlatform}, routeid});

        routeid = dv.routeid;
        stop = nextStop;
    }

//    Time    time = dv.time;
//    auto    pred = p[v];
//    while (pred != v && v != u) {
//        auto    dpred = d.at(pred);
////        auto    e = boost::edge(pred, v, graph_);
//        auto    el = edge_list(pred, v);
//        auto    eit =
//            std::min_element(el.cbegin(), el.cend(), [pred, &dpred, &w, &combine, &compare](
//                EdgeDesc e1, EdgeDesc e2) {

//            return compare(combine(dpred, w[e1]), combine(dpred, w[e2]));
//        });
//        auto        section = graph_[*eit];
//        const auto& to = section.to;
//        const auto& routeid = section.routeid;
//        rv.push_back(
//            Node{
//                {stop, time, lines_.getPlatform(routeid, stop)},
//                {to, lines_.getArriveTime(day, routeid, stop, time, to), lines_.getPlatform(routeid, to)},
//                routeid});
//        stop = graph_[pred];
//        time = dpred.time;
//        v = pred;
//        pred = p[v];
//    }

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
    auto    timeline = lines_.getStopTimesByRoute(day, to);
    Time    lastLeave{minusInf};
    Time    lastArrive{plusInf};
    for (const auto& timeByRoute: timeline) {
        auto    nlist = planFromArrive(day, from, to, timeByRoute, details, delay);
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


