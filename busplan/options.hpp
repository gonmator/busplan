#pragma once
#ifndef OPTIONS_HPP
#define OPTIONS_HPP

#include <istream>
#include <ostream>
#include <map>
#include <string>
#include <vector>

#include <boost/program_options/errors.hpp>
#include <boost/program_options/variables_map.hpp>

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
    getTable
};

std::string toString(Command command);
std::ostream& listCommands(std::ostream&);

class MissingOption: public boost::program_options::error_with_option_name {
public:
    using OptionNameList = std::vector<std::string>;
    MissingOption(const std::string& command, const OptionNameList& optionNames):
        boost::program_options::error_with_option_name(
            "command '%command%' requires options %option-list%") {

        set_substitute("command", command);
        set_substitute("option-list", toString(optionNames));
    }

    MissingOption(const std::string& command, const std::string& optionName):
        boost::program_options::error_with_option_name(
            "command '%command%' requires option '%canonical_option%'", optionName) {

        set_substitute("command", command);
    }
private:
    static std::string toString(const OptionNameList&);
    static std::string quote(const std::string&);
};

std::istream& operator>>(std::istream&, Command&);
void validate(const boost::program_options::variables_map&);



#endif // OPTIONS_HPP
