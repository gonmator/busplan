#include <boost/lexical_cast.hpp>

#include "details.hpp"

std::istream& operator>>(std::istream& is, Details& details) {
    std::string str;
    is >> str;
    if (str == "steps") {
        details = Details::steps;
    } else if (str == "transfers") {
        details = Details::transfers;
    } else if (str == "ends") {
        details = Details::ends;
    } else {
        throw boost::bad_lexical_cast{};
    }

    return is;
}

std::ostream& operator<<(std::ostream& os, Details details) {
    switch (details) {
    case Details::steps:
        return os << "steps";
    case Details::transfers:
        return os << "transfers";
    case Details::ends:
        return os << "ends";
    }
    throw boost::bad_lexical_cast{};
}

