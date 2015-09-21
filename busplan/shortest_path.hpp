#pragma once
#ifndef SHORTEST_PATH_HPP
#define SHORTEST_PATH_HPP

#include <algorithm>
#include <queue>

#include <boost/graph/dijkstra_shortest_paths.hpp>
#include <boost/graph/graph_traits.hpp>


template <
    typename Weight, typename Distance,
    typename CompareFunction, typename CombineFunction, typename AssignFunction>
bool relaxed(
    const Weight&   w,
    const Distance& du,
    Distance&       dv,
    CompareFunction cmp,
    CombineFunction cmb,
    AssignFunction  asg) {

    auto    nd = cmb(du, w);
    if (cmp(nd, dv)) {
        asg(dv, nd);
        return true;
    }
    return false;
}


template <
    typename IncidencesGraph, typename WeightMap, typename DistanceMap,
    typename CompareFunction, typename CombineFunction, typename AssignFunction, typename Visitor>
void mod_dijkstra_shortest_path_no_init(
    const IncidencesGraph&                                              g,
    typename boost::graph_traits<IncidencesGraph>::vertex_descriptor    s,
    const WeightMap&                                                    w_map,
    DistanceMap&                                                        d_map,
    CompareFunction                                                     cmp,
    CombineFunction                                                     cmb,
    AssignFunction                                                      asg,
    Visitor                                                             vis =
        boost::dijkstra_visitor<boost::null_visitor>{}) {

    using vertex_descriptor = typename boost::graph_traits<IncidencesGraph>::vertex_descriptor;
    using edge_descriptor = typename boost::graph_traits<IncidencesGraph>::edge_descriptor;

    std::queue<vertex_descriptor>   q;
    q.push(s); vis.discover_vertex(s, g);
    while (!q.empty()) {
        auto    u = q.front();
        q.pop(); vis.examine_vertex(u, g);
        auto    erg = boost::out_edges(u, g);
        std::for_each(erg.first, erg.second, [&](edge_descriptor e) {
            vis.examine_edge(e, g);
            auto        u = boost::source(e, g);
            auto        v = boost::target(e, g);
            auto        w = w_map.at(e);
            const auto& du = d_map.at(u);
            auto&       dv = d_map[v];
            if (relaxed(w, du, dv, cmp, cmb, asg)) {
                vis.edge_relaxed(e, g);
                q.push(v); vis.discover_vertex(v, g);
            } else {
                vis.edge_not_relaxed(e, g);
            }
        });
    }
}


template <
    typename VertexListGraph, typename WeightMap, typename DistanceMap,
    typename CompareFunction, typename CombineFunction, typename AssignFunction, typename Visitor>
void mod_dijkstra_shortest_path(
    const VertexListGraph&                                              g,
    typename boost::graph_traits<VertexListGraph>::vertex_descriptor    s,
    const WeightMap&                                                    w_map,
    DistanceMap&                                                        d_map,
    CompareFunction                                                     cmp,
    CombineFunction                                                     cmb,
    AssignFunction                                                      asg,
    typename DistanceMap::mapped_type                                   s_dist,
    typename DistanceMap::mapped_type                                   inf_dist,
    Visitor                                                             vis =
        boost::dijkstra_visitor<boost::null_visitor>{}) {

    using vertex_descriptor = typename boost::graph_traits<VertexListGraph>::vertex_descriptor;
    auto    vrg = boost::vertices(g);
    std::for_each(vrg.first, vrg.second, [&g, inf_dist, &d_map, &vis](vertex_descriptor v) {
        d_map[v] = inf_dist; vis.initialize_vertex(v, g);
    });
    d_map[s] = s_dist;

    mod_dijkstra_shortest_path_no_init(
        g, s, w_map, d_map, std::move(cmp), std::move(cmb), std::move(asg), std::move(vis));
}


#endif // SHORTEST_PATH_HPP
