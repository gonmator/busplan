TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += \
    busplan/bus_network.cpp \
    busplan/config.cpp \
    busplan/main.cpp \
    busplan/options.cpp \
    utility/ini_doc.cpp \
    busplan/day.cpp \
    busplan/details.cpp \
    busplan/line.cpp \
    busplan/lines.cpp \
    busplan/fragment.cpp \
    busplan/schedule.cpp \
    busplan/time_line.cpp \
    busplan/stop.cpp \
    busplan/time_table.cpp \
    busplan/logger.cpp

HEADERS += \
    busplan/lines.hpp \
    busplan/bus_network.hpp \
    busplan/walking.hpp \
    busplan/time.hpp \
    busplan/stop.hpp \
    busplan/config.hpp \
    busplan/route.hpp \
    busplan/line.hpp \
    busplan/schedule.hpp \
    busplan/algorithm.hpp \
    busplan/options.hpp \
    busplan/time_line.hpp \
    utility/literal.hpp \
    utility/ini_doc.hpp \
    busplan/day.hpp \
    busplan/details.hpp \
    busplan/fragment.hpp \
    busplan/time_table.hpp \
    busplan/logger.hpp


unix|win32: LIBS += -lboost_program_options

OTHER_FILES += \
    networks/lux/7xx.lines.cfg \
    networks/lux/walking.cfg \
    networks/lux/stops.cfg \
    networks/lux/1xx.lines.cfg \
    networks/lux/busplan.cfg \
    networks/lux/old.avl.lines.cfg \
    networks/lux/avl.lines.cfg \
    networks/lux/27.cfg \
    networks/busplan.cfg \
    networks/lux/18.cfg \
    networks/lux/26.cfg \
    networks/lux/13.cfg \
    networks/lux/22.cfg \
    networks/lux/8.cfg \
    networks/lux/28.cfg
