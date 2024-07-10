#include "sharedmemorymanager.h"

#include <cstring>
#include <fcntl.h>
#include <iostream>
#include <semaphore.h>
#include <sys/mman.h>
#include <unistd.h>

constexpr int SIZE = 2 * 1024 * 1024 + 100;

SharedMemoryManager::SharedMemoryManager(const std::string& semaphorePreffixName, const char* SharedObjName)
{
    try {
        if (OpenSharedMemory(semaphorePreffixName, SharedObjName)) {
            type = Type::reader;
        } else {
            std::cout << std::this_thread::get_id() << " may be writer " << std::endl;
            static std::string writerName = semaphorePreffixName + '2';
            m_writerSem                   = sem_open(writerName.c_str(), O_CREAT, S_IRUSR | S_IWUSR, 1);
            if (m_writerSem != SEM_FAILED) {
                type = Type::writer;
            }
        }

        m_shmPtr = static_cast<char*>(mmap(0, SHM_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, m_shmFd, 0));
        if (m_shmPtr == MAP_FAILED) {
            throw std::bad_alloc();
        }
    } catch (...) {
        type = Type::none;
    }
}

bool SharedMemoryManager::InitializeSharedMemory(const std::string& semaphoreName, bool isCreator)
{
    m_memorySem = std::make_unique<SemWrapper>(semaphoreName, 1);
    SemMutexWrapper mutex(m_memorySem);

    if (isCreator && ftruncate(m_shmFd, SIZE) == -1) {
        throw std::system_error(errno, std::system_category(), "Error setting size for shared memory");
    }

    return isCreator;
}

bool SharedMemoryManager::OpenSharedMemory(const std::string& semaphorePreffixName, const char* SharedMemoryName)
{
    std::string semaphoreName = semaphorePreffixName + '1';

    int maxAttempts  = 5;
    int currAttempts = 0;

    while (currAttempts < maxAttempts) {
        m_shmFd = shm_open(SharedMemoryName, O_CREAT | O_EXCL | O_RDWR, 0666);
        if (m_shmFd != -1) {
            return InitializeSharedMemory(semaphoreName, true);
        }
        if (errno != EEXIST) {
            throw std::logic_error(std::string("Error creating/opening shared memory: ") + strerror(errno));
        }
        m_shmFd = shm_open(SharedMemoryName, O_RDWR, 0666);
        if (m_shmFd != -1) {
            return InitializeSharedMemory(semaphoreName, false);
        }
        ++currAttempts;

        sleep(1);
    }

    throw std::runtime_error("Failed to open shared memory after maximum attempts");
}

toSend SharedMemoryManager::GetBufferByIndex(size_t index)
{
    // add out of bound check
    toSend buffer = toSend(new (m_shmPtr + sizeof(Message) * 2 + index * sizeof(Buffer)) Buffer(index));
    return buffer;
}

Message* SharedMemoryManager::GetQueueByIndex(const size_t index) const
{
    return reinterpret_cast<Message*>(m_shmPtr + sizeof(Message) * index);
}

SharedMemoryManager::~SharedMemoryManager()
{
    std::cout << std::this_thread::get_id() << " ~SharedMemoryWrapper" << std::endl;

    if (munmap(m_shmPtr, SHM_SIZE) == -1) {
        perror("munmap");
        exit(1);
    }

    if (close(m_shmFd) == -1) {
        perror("close");
        exit(1);
    }
}

Type SharedMemoryManager::WhoAmI() const
{
    return type;
}
