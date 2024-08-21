#pragma once

#include <string>
#include <utility>

/// A RAII class to track other classes lifetime
class Logger {
public:
    Logger(std::string className, const void* ptr);
    virtual ~Logger();

protected:
    void logCreation(const void* ptr) const;
    void logDestruction(const void* ptr) const;

private:
    const void* m_ptr;
    std::string m_className;
};