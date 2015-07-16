#pragma once
#ifndef LOGGER_HPP
#define LOGGER_HPP

#include <fstream>
#include <map>
#include <memory>
#include <string>

class Log {
public:
    Log(const std::string& logfile);

    Log() = default;
    Log(Log&&) = default;
    Log& operator=(Log&&) = default;
    ~Log() = default;

    Log(const Log&) = delete;
    Log& operator=(const Log&) = delete;

    template <typename T>
    std::ostream& operator<<(T value) {
        return ofs_ << value;
    }
    std::ostream& operator<<(std::ostream& (*func)(std::ostream&)) {
        return func(ofs_);
    }

private:
    std::ofstream   ofs_;
};

class Logger {
public:
    Log& create(const std::string& name, const std::string& logfile);
    bool exists(const std::string& name) const;
    Log& log(const std::string& name) const;

private:
    using LoggerMap = std::map<std::string, std::unique_ptr<Log>>;
    static LoggerMap& loggerMap();
};



#endif // LOGGER_HPP
