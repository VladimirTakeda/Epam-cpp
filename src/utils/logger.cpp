#include "logger.h"

#include <iostream>

Logger::Logger(std::string className)
    : m_className(std::move(className))
{
    std::cout << "Object of class " << m_className << " created: " << this << std::endl;
}
Logger::~Logger()
{
    std::cout << "Object of class " << m_className << " destroyed: " << this << std::endl;
}
