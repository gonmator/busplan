#include <map>
#include <stdexcept>
#include <string>
#include <vector>

#include "options.hpp"

namespace {

const std::map<std::string, Command>  cmdMap = {
    {"help", Command::help},
    {"get-line", Command::getLines},
    {"get-plan", Command::getPlan},
    {"get-route", Command::getRoutes},
    {"get-table", Command::getTable}
};

}

std::istream& operator>>(std::istream& is, Command& command) {
    std::string str;
    is >> str;
    command = cmdMap.at(str);
    return is;
}

std::ostream& listCommands(std::ostream& os) {
    for (const auto& v: cmdMap) {
        os << "  " << v.first << std::endl;
    }
    return os;
}

void validate(const boost::program_options::variables_map& vm) {
    const auto& command = vm.at("command").as<Command>();
    auto        checkForMissing =
        [&vm](const std::string& command, const MissingOption::OptionNameList& expectedOptionNames) {

        MissingOption::OptionNameList   missingOptionNames;
        for(const auto& eOptionName: expectedOptionNames) {
            if (!vm.count(eOptionName)) {
                missingOptionNames.push_back(eOptionName);
            }
        }
        if (!missingOptionNames.empty()) {
            throw MissingOption(command, missingOptionNames);
        }
    };

    switch (command) {
    case Command::help:
        break;
    case Command::getPlan:
        checkForMissing("get-plan", {"from", "to", "arrive"});
        break;
    case Command::getLines:
        break;
    case Command::getRoutes:
        break;
    case Command::getTable:
        checkForMissing("get-plan", {"from", "to"});
        break;
    }
}



