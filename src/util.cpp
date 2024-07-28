#include "util.h"

#include <cstring>
#include <fcntl.h>
#include <filesystem>
#include <iostream>
#include <utility>

SemWrapper::SemWrapper(std::string semName, int initialValue, DeletePolicy m_policy)
    : m_semName(std::move(semName))
    , m_semaphore(nullptr)
    , m_created(false)
    , m_policy(m_policy)
{
    m_semaphore = sem_open(m_semName.c_str(), O_CREAT | O_EXCL, S_IRUSR | S_IWUSR, initialValue);
    if (m_semaphore == SEM_FAILED) {
        if (errno == EEXIST) {
            m_semaphore = sem_open(m_semName.c_str(), 0);
            if (m_semaphore == SEM_FAILED) {
                throw std::runtime_error("Error opening existing semaphore: " + std::string(strerror(errno)));
            }
        } else {
            throw std::runtime_error("Error creating semaphore: " + std::string(strerror(errno)));
        }
    } else {
        m_created = true;
    }
}

bool SemWrapper::IsCreator() const
{
    return m_created;
}

/// TODO: who is responsible for eresure?
/// In default situation, when everything is file it should be reader because reader acts first
SemWrapper::~SemWrapper()
{
    sem_close(m_semaphore);
    if (m_policy == DeletePolicy::Delete)
        sem_unlink(m_semName.data());
    if (m_policy == DeletePolicy::DecideYourSelf && m_created)
        sem_unlink(m_semName.data());
}

sem_t* SemWrapper::Get() const
{
    return m_semaphore;
}

SemMutexWrapper::SemMutexWrapper(std::unique_ptr<SemWrapper>& semWrapper)
    : m_Sem(semWrapper)
{
    sem_wait(m_Sem->Get());
}

SemMutexWrapper::~SemMutexWrapper()
{
    sem_post(m_Sem->Get());
}

void EraseFile(const char* fileName)
{
    try {
        if (std::filesystem::remove(fileName)) {
            std::cout << "File deleted successfully" << std::endl;
        } else {
            std::cout << "File not found or could not be deleted" << std::endl;
        }
    } catch (const std::filesystem::filesystem_error& e) {
        std::cerr << "Filesystem error: " << e.what() << std::endl;
    }
}
