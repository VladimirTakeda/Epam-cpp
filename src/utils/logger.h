#pragma once

#include <string>
#include <utility>

/// A RAII class to track other classes lifetime
/// You should inherit from this class
class Logger {
public:
    Logger(std::string className);
    ~Logger();

private:
    std::string m_className;
};