#pragma once
#ifndef OPTIONS_HPP
#define OPTIONS_HPP

#include <map>
#include <string>
#include <vector>

#include "../utility/literal.hpp"
#include "time.hpp"

using OptionValue = Utility::Literal;
using Options = std::map<std::string, OptionValue>;
using Option = Options::value_type;

enum class Command {
    help,

    getPlan,
    getLines,
    getRoutes,

    invalid
};

Options parseCommandLine(int argc, const char * const argv[]);
Command getCommand(const Options& options);

#endif // OPTIONS_HPP
