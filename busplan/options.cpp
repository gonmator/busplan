#include <stdexcept>

#include "options.hpp"

Options parseCommandLine(int argc, const char * const argv[]) {
    Options     rv;
    std::string name;
    while (--argc) {
        std::string arg = *++argv;

        if (arg[0] == '-') {
            name = arg.substr(1);
        } else {
            if (!rv.emplace(name, arg).second) {
                throw std::invalid_argument("option already set");
            }
        }
    }

    return rv;
}

Command getCommand(const Options& options) {

    const auto& cmdstr = options.at("");
    if (cmdstr == "help") {
        return Command::help;
    }
    if (cmdstr == "get-plan") {
        if (options.count("from") && options.count("to") && options.count("arrive")) {
            return Command::getPlan;
        }
    }
    if (cmdstr == "get-lines") {
        return Command::getLines;
    }
    if (cmdstr == "get-routes") {
//        if (options.count("line")) {
            return Command::getRoutes;
//        }
    }

    return Command::invalid;
}


