#pragma once

#include <semaphore.h>

enum class Type : uint8_t {
    reader = 0,
    writer = 1,
    none = 2
};

/// Posix Semaphore wrapper that does linking to and unlinking from current process adress space
class SemWrapper {
public:
    explicit SemWrapper(std::string semName, int intialValue);
    ~SemWrapper();
    sem_t* Get() const;

private:
    std::string m_semName;
    sem_t *m_semaphore;
};

void EraseFile(const char *fileName);