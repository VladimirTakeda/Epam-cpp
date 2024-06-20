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
            std::cout << std::this_thread::get_id() << " may be writer " << std::endl;
            static std::string writerName = semaphorePreffixName + '2';
            m_writerSem = sem_open(writerName.c_str(), O_CREAT, S_IRUSR | S_IWUSR, 1);
            if (m_writerSem != SEM_FAILED) {
                type = Type::writer;
            }
        }

        m_shmPtr = static_cast<char *>(mmap(0, SHM_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, m_shmFd, 0));
        if (m_shmPtr == MAP_FAILED) {
            throw std::bad_alloc();
        }
    }
    catch (...) {
        type = Type::none;
    }
}

bool SharedMemoryManager::OpenSharedMemory(const std::string& semaphorePreffixName, const char *SharedMemoryName) {
    std::string semaphoreName = semaphorePreffixName + '1';

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
            std::cout << std::this_thread::get_id() << " 123 " << std::endl;
            throw std::logic_error(std::string("Error creating/opening shared memory: ") + strerror(errno));
        }
    }

    m_memorySem = sem_open(semaphoreName.data(), O_CREAT, S_IRUSR | S_IWUSR, 1);

    if (m_memorySem == SEM_FAILED) {
        std::cout << std::this_thread::get_id() << " 124 " << semaphoreName << std::endl;
        throw std::logic_error(std::string("Error creating/opening semaphore: ") + strerror(errno));
    }

    if (sem_wait(m_memorySem) == -1) {
        std::cout << std::this_thread::get_id() << " 125 " << std::endl;
        throw std::logic_error(std::string("Error waiting on semaphore: ") + strerror(errno));
    }

    try {
        if (is_creator) {
            if (ftruncate(m_shmFd, SIZE) == -1) {
                std::cout << std::this_thread::get_id() << " 126 " << std::endl;
                throw std::runtime_error(strerror(errno));
            }
        }
    } catch (const std::exception &ex) {
        sem_post(m_memorySem);
        std::cout << std::this_thread::get_id() << " 127 " << std::endl;
        throw std::logic_error(std::string("Error setting size for shared memory: ") + ex.what());
    }

    sem_post(m_memorySem);

    return is_creator;
}

toSend SharedMemoryManager::GetBufferByIndex(size_t index) {
    // add out of bound check
    toSend buffer = toSend(new(m_shmPtr + sizeof(uint32_t) * 2 + index * sizeof(Buffer)) Buffer());
    return buffer;
}

uint32_t* SharedMemoryManager::GetQueueByIndex(const size_t index) const{
    return reinterpret_cast<uint32_t*>(m_shmPtr + sizeof(uint32_t) * index);
}

SharedMemoryManager::~SharedMemoryManager() {
    std::cout << " ~SharedMemoryWrapper" << std::endl;

    std::cout << "Close semaphores" << std::endl;

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
