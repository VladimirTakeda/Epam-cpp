#include <filesystem>
#include <iostream>
#include <utility>
#include <cstring>

#include <fcntl.h>

#include "util.h"

SemWrapper::SemWrapper(std::string semName, int initialValue) : m_semName(std::move(semName)) {
    m_semaphore = sem_open(m_semName.data(), O_CREAT, S_IRUSR | S_IWUSR, initialValue);
    if (m_semaphore == SEM_FAILED) {
        throw std::logic_error(std::string("Error creating/opening semaphore: ") + strerror(errno));
    }
}

/// TODO: who is responsible for eresure?
/// In default situation, when everything is file it should be reader because reader acts first
SemWrapper::~SemWrapper() {
    sem_close(m_semaphore);
    //sem_unlink(m_semName.data());
}

sem_t* SemWrapper::Get() const {
    return m_semaphore;
}

void EraseFile(const char *fileName) {
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
