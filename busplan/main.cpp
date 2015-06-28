#include <algorithm>
#include <cassert>
#include <fstream>
#include <iostream>

#include <boost/program_options/cmdline.hpp>
#include <boost/program_options/errors.hpp>
#include <boost/program_options/options_description.hpp>
#include <boost/program_options/parsers.hpp>
#include <boost/program_options/positional_options.hpp>
#include <boost/program_options/value_semantic.hpp>
#include <boost/program_options/variables_map.hpp>

#include "bus_network.hpp"
#include "config.hpp"
#include "day.hpp"
#include "details.hpp"
#include "lines.hpp"
#include "options.hpp"

std::string MissingOption::toString(const OptionNameList& optionNames) {
    assert(!optionNames.empty());

    std::string rv{quote(optionNames.at(0))};
    std::for_each(optionNames.cbegin() + 1, optionNames.cend(), [&rv](const std::string& optionName) {
        rv.append(", ").append(quote(optionName));
    });
    return rv;
}

std::string MissingOption::quote(const std::string& str) {
    return std::string("'").append(str).append("'");
}

void getConfig(Utility::IniDoc& config, const std::string& fname) {
    std::ifstream   cfgf(fname);
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
            config.merge(iconfig, Utility::IniDoc::DuplicateAction::combineProperty);
        }
    }
}

int main(int argc, char *argv[])
{
    namespace po = boost::program_options;

    std::string fromStop, toStop;
    Time        arriveTime;
    Day         day;
    Command     cmd;
    Details     details;

    po::options_description command_desc("Command");
    command_desc.add_options()
        ("command",
            po::value<Command>(&cmd)->value_name("command")->required(), "{help|get-plan|get-lines|get-routes|get-table}");
    po::options_description option_desc("Options");
    option_desc.add_options()
        ("from", po::value<std::string>(&fromStop)->value_name("BUS-STOP"))
        ("to", po::value<std::string>(&toStop)->value_name("BUS-STOP"))
        ("arrive", po::value<Time>(&arriveTime)->value_name("TIME"))
        ("date", po::value<Day>(&day)->value_name("DATE")->default_value(Day{"today"}))
        ("details", po::value<Details>(&details)->value_name("DETAILS")->default_value(Details::steps))
        ;
    po::positional_options_description  cmdDesc;
    cmdDesc.add("command", 1);
    po::options_description desc;
    desc.add(command_desc).add(option_desc);
    po::variables_map   vm;
    try {
        po::store(po::command_line_parser(argc, argv).options(desc).positional(cmdDesc).run(), vm);
        po::notify(vm);
        validate(vm);
    } catch (const po::error& e) {
        std::string pfname(argv[0]);
        pfname.erase(0, pfname.find_last_of("/\\") + 1);
        std::cerr << e.what() << std::endl;
        std::cerr << "(Try \"" << pfname << " help\")" << std::endl;
        return 1;
    }

    if (cmd == Command::help) {
        std::cout << "Busplan" << std::endl << std::endl;
        std::cout << "Usage:" << std::endl;
        std::cout << "  busplan <command> [options]" << std::endl << std::endl;
        std::cout << "Commands:" << std::endl;
        listCommands(std::cout) << std::endl;
        std::cout << option_desc << std::endl;
        return 0;
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

    if (!fromStop.empty()) {
        fromStop = checkStop(stopdescs, fromStop);
        if (fromStop.empty()) {
            std::cerr << "invalid from stop" << std::endl;
        }
    }
    if (!toStop.empty()) {
        toStop = checkStop(stopdescs, toStop);
        if (toStop.empty()) {
            std::cerr << "invalid to stop" << std::endl;
        }
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
//        LineNames   linesn;
//        if (options.count("line")) {
//            linesn.push_back(options.at("line"));
//        } else {
//            linesn = busNetwork.getLineNames();
//        }
        LineNames  linesn{busNetwork.getLineNames()};
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
        auto    routelist = busNetwork.planFromArrive(day, fromStop, toStop, arriveTime, details);

        std::cout << "From\tLeave\tRoute\tTo\tArrive" << std::endl;
        for (const auto& node: routelist) {
            //  from
            if (node.from.platform.empty()) {
                std::cout << stopdescs.at(node.from.stop)[0] << "\t";
            } else {
                std::cout << node.from.platform << "\t";
            }
            //  leave
            std::cout << toString(node.from.time) << "\t";
            //  route
            std::cout << node.routeid.linen;
            if (!node.routeid.routen.empty()) {
                std::cout << " [" << node.routeid.routen << "]";
            }
            std::cout << "\t";
            //  to
            if (node.to.platform.empty()) {
                std::cout << stopdescs.at(node.to.stop)[0] << "\t";
            } else {
                std::cout << node.to.platform;
            }
            //  arrive
            std::cout << toString(node.to.time);

            std::cout << std::endl;
        }
    }

    if (cmd == Command::getTable) {
        auto    table = busNetwork.table(day, fromStop, toStop, details);

        for (const auto& nodeList: table) {
            for (const auto& node: nodeList) {
                //  from
                if (node.from.platform.empty()) {
                    std::cout << stopdescs.at(node.from.stop)[0] << "\t";
                } else {
                    std::cout << node.from.platform << "\t";
                }
                //  leave
                std::cout << toString(node.from.time) << "\t";
                //  route
                std::cout << node.routeid.linen;
                if (!node.routeid.routen.empty()) {
                    std::cout << " [" << busNetwork.routeName(node.routeid) << "]";
                }
                std::cout << "\t";
                //  to
                if (node.to.platform.empty()) {
                    std::cout << stopdescs.at(node.to.stop)[0] << "\t";
                } else {
                    std::cout << node.to.platform << "\t";
                }
                //  arrive
                std::cout << toString(node.to.time) << "\t";
            }
            std::cout << std::endl;
        }
    }


    return 0;
}

