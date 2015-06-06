#pragma once
#ifndef DETAILS_HPP
#define DETAILS_HPP

#include <istream>
#include <ostream>

enum class Details {
    steps,
    transfers,
    ends
};

std::istream& operator>>(std::istream&, Details&);
std::ostream& operator<<(std::ostream&, Details);

#endif // DETAILS_HPP
