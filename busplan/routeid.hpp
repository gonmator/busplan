#pragma once
#ifndef ROUTEID_HPP
#define ROUTEID_HPP

#include <string>
#include <vector>

using LineName = std::string;
using LineNames = std::vector<LineName>;
using RouteName = std::string;
using RouteNames = std::vector<RouteName>;

struct RouteId {
    RouteId(LineName ln, RouteName rn): linen{std::move(ln)}, routen{std::move(rn)} {}
    RouteId() = default;

    LineName    linen;
    RouteName   routen;
};
inline bool operator ==(const RouteId& rida, const RouteId& ridb) {
    return rida.linen == ridb.linen && rida.routen == ridb.routen;
}
inline bool operator !=(const RouteId& rida, const RouteId& ridb) {
    return !(rida == ridb);
}
inline bool operator <(const RouteId& rida, const RouteId& ridb) {
    return rida.linen < ridb.linen || (rida.linen == ridb.linen && rida.routen < ridb.routen);
}

#endif // ROUTEID_HPP
