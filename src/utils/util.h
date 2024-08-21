#pragma once

#include "logger.h"

#include <memory>
#include <semaphore.h>

enum class Type : uint8_t { reader = 0, writer = 1, none = 2 };

enum class DeletePolicy : uint8_t { Delete = 0, DontDelete = 1 };

/// Posix Semaphore wrapper that does linking to and unlinking from current process adress space
class SemWrapper : public Logger {
public:
    explicit SemWrapper(std::string semName, int intialValue, DeletePolicy policy);
    [[nodiscard]] bool IsCreator() const;
    [[nodiscard]] sem_t* Get() const;
    ~SemWrapper();

private:
    std::string m_semName;
    sem_t* m_semaphore;
    bool m_created;
    DeletePolicy m_policy;
};

class SemMutexWrapper {
public:
    explicit SemMutexWrapper(std::unique_ptr<SemWrapper>& semWrapper);
    ~SemMutexWrapper();

private:
    std::unique_ptr<SemWrapper>& m_Sem;
};

void EraseFile(const char* fileName);