#include <cstring>
#include <iostream>

#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <semaphore.h>

#include "sharedmemorymanager.h"

constexpr int SIZE = 2 * 1024 * 1024 + 100;

SharedMemoryManager::SharedMemoryManager(const std::string& semaphorePreffixName, const char *SharedObjName) {
    try {
        if (OpenSharedMemory(semaphorePreffixName, SharedObjName)) {
            type = Type::reader;
        }
        else {
            static std::string writerName = semaphorePreffixName + '2';
            m_writerSem = sem_open(writerName.c_str(), O_CREAT, 0666, 1);
            if (m_writerSem != SEM_FAILED) {
                type = Type::writer;
            }
        }
    }
    catch (...) {
        type = Type::none;
    }
}

bool SharedMemoryManager::OpenSharedMemory(const std::string& semaphorePreffixName, const char *SharedMemoryName) {
    static std::string semaphoreName = semaphorePreffixName + '1';

    bool is_creator = false;

    int max_attempts = 5;
    int attempts = 0;

    while (attempts < max_attempts){
        m_shmFd = shm_open(SharedMemoryName, O_CREAT | O_EXCL | O_RDWR, 0666);
        if (m_shmFd != -1) {
            is_creator = true;
            break;
        }
        if (errno == EEXIST) {
            m_shmFd = shm_open(SharedMemoryName, O_RDWR, 0666);
            if (m_shmFd != -1) {
                break;
            }
            attempts++;
            sleep(1);
        } else {
            throw std::logic_error(std::string("Error creating/opening shared memory: ") + strerror(errno));
        }
    }

    m_memorySem = sem_open(semaphoreName.c_str(), O_CREAT, 0666, 1);

    if (m_memorySem == SEM_FAILED) {
        throw std::logic_error(std::string("Error creating/opening semaphore: ") + strerror(errno));
    }

    if (sem_wait(m_memorySem) == -1) {
        throw std::logic_error(std::string("Error waiting on semaphore: ") + strerror(errno));
    }

    try {
        if (is_creator) {
            if (ftruncate(m_shmFd, SIZE) == -1) {
                throw std::runtime_error(strerror(errno));
            }
        }
    } catch (const std::exception &ex) {
        sem_post(m_memorySem);
        throw std::logic_error(std::string("Error setting size for shared memory: ") + ex.what());
    }

    sem_post(m_memorySem);

    return is_creator;
}

toSend SharedMemoryManager::GetBufferByIndex(size_t index) {
    // add out of bound check
    toSend buffer = toSend(new(m_shmPtr + sizeof(int) + index * sizeof(Buffer)) Buffer());
    currIdx = 1 - currIdx;
    return buffer;
}

char* SharedMemoryManager::GetQueueByIndex(size_t index) const{
    static_assert(index > 2);
    return m_shmPtr + sizeof(int) * index;
}

SharedMemoryManager::~SharedMemoryManager() {
    std::cout << " ~SharedMemoryWrapper" << std::endl;
    // if it's the last running process we need to close all the semaphores
    sem_wait(m_counterSem);
    int *counter = reinterpret_cast<int *>(m_shmPtr);
    --(*counter);
    int count = *counter;
    sem_post(m_counterSem);

    if (count == 0) {
        std::cout << "Close semaphores";
        sem_close(m_releaseSem);
        sem_close(m_captureSem);
        sem_close(m_counterSem);

        sem_unlink(SEM_NAME_READER.data());
        sem_unlink(SEM_NAME_WRITER.data());
        sem_unlink(SEM_NAME_COUNTER.data());
    }

    if (m_memorySem) {
        sem_close(m_memorySem);
    }

    if (m_writerSem) {
        sem_close(m_memorySem);
    }

    if (munmap(m_shmPtr, SHM_SIZE) == -1) {
        perror("munmap");
        exit(1);
    }

    if (close(m_shmFd) == -1) {
        perror("close");
        exit(1);
    }
}

Type SharedMemoryManager::WhoAmI() const {
    return type;
}

void SharedMemoryManager::SetCurrType() {
    sem_wait(m_counterSem);
    int *counter = reinterpret_cast<int *>(m_shmPtr);
    type = static_cast<Type>(*counter % 2);
    ++(*counter);
    sem_post(m_counterSem);
}
