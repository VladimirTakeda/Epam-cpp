#include "logger.h"

#include <iostream>

Logger::Logger(std::string className, const void* ptr)
    : m_ptr(ptr)
    , m_className(std::move(className))
{
    std::cout << "Object of class " << m_className << " created: " << m_ptr << std::endl;
}
Logger::~Logger()
{
    std::cout << "Object of class " << m_className << " destroyed: " << m_ptr << std::endl;
}
