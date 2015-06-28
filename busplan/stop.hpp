#pragma once
#ifndef STOP_HPP
#define STOP_HPP

#include <map>
#include <string>
#include <set>
#include <vector>

using Stop = std::string;
using Stops = std::vector<Stop>;
using StopSet = std::set<Stop>;
using StopDescriptions = std::map<Stop, std::vector<std::string>>;
using StopDescription = StopDescriptions::value_type;

std::string checkStop(const StopDescriptions& stopDescriptions, const std::string& stopName);

#endif // STOP_HPP
