#pragma once
#ifndef CONFIG_HPP
#define CONFIG_HPP

#include "../utility/ini_doc.hpp"
#include "lines.hpp"

void read(const Utility::IniDoc::Doc& cfg, Lines& lines, StopDescriptions &sds);

#endif // CONFIG_HPP
