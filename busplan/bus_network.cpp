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

struct Point {
    Stop    stop;
    Time    time;
};

struct Step {
    Point   point;
    RouteId routeId;
};

class GraphDistance {
public:
    using Distances = std::map<RouteId, Step>;
    using Distance = Distances::value_type;

    GraphDistance() = default;
    GraphDistance(const GraphDistance& gd) = default;
    GraphDistance(GraphDistance&& gd) = default;
    explicit GraphDistance(const Step& step): distances_{} {
        distances_[step.routeId] = Step{step.point, RouteId{}};
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

    const Step& at(const RouteId& routeid) const {
        return distances_.at(routeid);
    }
    Step& operator[](const RouteId& routeid) {
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
        if (tcomp(d1.second.point.time, it2->second.point.time)) {
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
        auto    arrive = d.second.point.time - tadjust(d.first, section.routeid);
        auto    leave = tget(section.routeid, section.from, section.to, arrive);
        if (leave == infinite) {
            continue;
        }
        auto    rit = rv.find(section.routeid);
        if (rit == rv.cend() || tcomp(leave, rit->second.point.time)) {
            rv[section.routeid] = Step{{section.to, leave}, d.first};
        }
    }
    return rv;
}

inline GraphDistance& assign(GraphDistance& gdl, const GraphDistance& gdr) {
    gdl.assign(gdr);
    return gdl;
}

std::ostream& operator<<(std::ostream& os, const GraphDistance::Distance& d) {
    os << d.first << " (time: " << toString(d.second.point.time) << ", through: " << d.second.routeId;
    os << ", to: " << d.second.point.stop << ")";
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

    return planFromArrive(day, from, to, TimeStep{arrive}, details, delay);
}

BusNetwork::NodeList BusNetwork::planFromArrive(
    Day day, const Stop& from, const Stop& to, TimeStep arriveBy, Details details, DifTime delay) {

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

    Step    step{Point{Stop{}, arriveBy.time}, arriveBy.routeid};
    mod_dijkstra_shortest_path(
        graph_, u, w, d, compare, combine, assign, GraphDistance{step}, GraphDistance{}, vis);

    Stop        stop = from;
    const auto& dv = d.at(stopMap_[stop]);
    auto        it = std::min_element(
        dv.cbegin(), dv.cend(), [](const GraphDistance::Distance& d1, const GraphDistance::Distance& d2) {

        return ::arrive_compare(d1.second.point.time, d2.second.point.time);
    });
    if (it == dv.cend()) {
        return rv;
    }
    auto    routeid = it->first;
    while (stop != to) {
        const auto& dv = d.at(stopMap_[stop]).at(routeid);
        auto        time = dv.point.time;
        const auto& platform = lines_.getPlatform(routeid, stop);
        auto        nextStop = dv.point.stop;
        auto        arrive = lines_.getArriveTime(day, routeid, stop, time, nextStop);
        const auto& nextPlatform = lines_.getPlatform(routeid, nextStop);

        rv.push_back(Node{{stop, time, platform}, {nextStop, arrive, nextPlatform}, routeid});

        routeid = dv.routeId;
        stop = nextStop;
    }

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
        std::cout << toString(time) << " ";
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


