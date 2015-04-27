#include <fstream>
#include <iostream>

#include "bus_network.hpp"
#include "config.hpp"
#include "lines.hpp"
#include "options.hpp"

void getConfig(Utility::IniDoc& config, const std::string& fname) {
    std::ifstream   cfgf(std::string{"../busplan/"}.append(fname));
    if (!cfgf.is_open()) {
        throw std::runtime_error(
            std::string{"Unable to open \""}.append(fname).append("\""));
    }
    cfgf >> config;
}

void resolveImports(Utility::IniDoc& config) {
    if (config.doc().at("").count("imports")) {
        const auto& importList = config.doc().at("").at("imports").items();
        for (const auto& importstr: importList) {
            Utility::IniDoc iconfig;
            getConfig(iconfig, importstr);
            resolveImports(iconfig);
            config.merge(iconfig);
        }
    }
}

int main(int argc, char *argv[])
{
    std::string fromStop, arriveStop;
    Time        arriveTime;

    auto    options = parseCommandLine(argc, argv);
    auto    cmd = getCommand(options);
    if (cmd == Command::invalid || cmd == Command::help) {
        int rv = 0;

        if (cmd == Command::invalid) {
            std::cerr << "Invalid command line arguments." << std::endl;
            rv = 3;
        }
        std::cout << "Usage: " << std::endl;
        std::cout << "    busplan get-plan -from <bus-stop> -to <bus-stop> -arrive <time>" << std::endl;
        std::cout << "    busplan get-lines" << std::endl;
        std::cout << "    busplan get-routes [-line <line>]" << std::endl;

        return rv;
    }

    Utility::IniDoc     config;
    Lines               lines;
    StopDescriptions    stopdescs;

    try {
        getConfig(config, "busplan.cfg");
        resolveImports(config);
        read(config.doc(), lines, stopdescs);
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return 2;
    }

    BusNetwork  busNetwork{std::move(lines)};

    if (cmd == Command::getLines) {
        auto    linesn = busNetwork.getLineNames();
        std::cout << "Lines:" << std::endl;
        for (const auto& linen: linesn) {
            std::cout << linen << " ";
        }
        std::cout << std::endl;
    }

    if (cmd == Command::getRoutes) {
        LineNames   linesn;
        if (options.count("line")) {
            linesn.push_back(options.at("line"));
        } else {
            linesn = busNetwork.getLineNames();
        }
        std::cout << "Lines / routes:" << std::endl;
        for (const auto& linen: linesn) {
            std::cout << linen << std::endl << "    ";
            auto    routesn = busNetwork.getRouteNames(linen);
            for (const auto& routen: routesn) {
                std::cout << routen << " ";
            }
            std::cout << std::endl;
        }
    }

    if (cmd == Command::getPlan) {
        const auto& from = options.at("from");
        const auto& to = options.at("to");
        auto        arrive = toTime(options.at("arrive"));
        auto        nodelist = busNetwork.planFromArrive(BusNetwork::Day::monToFri, from, to, arrive);

        std::cout << "Line.Route\tStop\tTime" << std::endl;
        for (const BusNetwork::Node& node: nodelist) {
            std::cout << node.routeid.linen;
            if (!node.routeid.routen.empty()) {
                std::cout << " [" << node.routeid.routen << "]";
            }
            std::cout << "\t" << stopdescs[node.stop][0];
            if (!node.platform.empty()) {
                std::cout << " [" << node.platform << "]";
            }
            std::cout << "\t" << toString(node.time) << std::endl;
        }
    }


    return 0;
}

