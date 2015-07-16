#include <cassert>

#include "logger.hpp"

std::unique_ptr<Log> makeLog() {
    return std::unique_ptr<Log>(new Log{});
}

std::unique_ptr<Log> makeLog(const std::string& name) {
    return std::unique_ptr<Log>(new Log{name});
}

Log::Log(const std::string& logfile): ofs_{logfile} {
    ofs_.exceptions(std::ofstream::failbit);
}

Log& Logger::create(const std::string& name, const std::string& logfile) {
    auto&   lm = loggerMap();
    auto    rv = lm.emplace(name, makeLog(logfile));
    if (!rv.second) {
        throw std::invalid_argument{"Logger::create: unable to register log"};
    }
    return *rv.first->second;
}

bool Logger::exists(const std::string& name) const {
    const auto& lm = loggerMap();
    return      lm.find(name) != lm.cend();
}

Log& Logger::log(const std::string& name) const {
    auto&   lm = loggerMap();
    auto    logit = lm.find(name);
    if (logit == lm.end()) {
        logit = lm.find("");
    }
    return *logit->second;
}

Logger::LoggerMap& Logger::loggerMap() {
    static LoggerMap    loggerMap_{};
    static bool         initialized = false;
    if (!initialized) {
        loggerMap_.emplace("", makeLog());
        initialized = true;
    }
    return loggerMap_;
}

