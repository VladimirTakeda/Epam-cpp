#include <cstring>
#include <iostream>

#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <semaphore.h>

#include "SharedMemoryWrapper.h"

SharedMemoryWrapper::SharedMemoryWrapper(const char *SharedObjName) {
    m_shmFd = shm_open(SharedObjName, O_RDWR, 0666);
    if (m_shmFd < 0) {
        throw std::bad_alloc();
    }

    m_shmPtr = static_cast<char *>(mmap(0, SHM_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, m_shmFd, 0));
    if (m_shmPtr == MAP_FAILED) {
        throw std::bad_alloc();
    }

    m_counterSem = sem_open(SEM_NAME_COUNTER.data(), O_CREAT, S_IRUSR | S_IWUSR, 1);
    sem_t *writer_sem = sem_open(SEM_NAME_WRITER.data(), O_CREAT, S_IRUSR | S_IWUSR, 0);
    sem_t *reader_sem = sem_open(SEM_NAME_READER.data(), O_CREAT, S_IRUSR | S_IWUSR, 2);

    if (m_counterSem == SEM_FAILED || writer_sem == SEM_FAILED || reader_sem == SEM_FAILED) {
        throw std::logic_error("failed to open semaphore");
    }

    SetCurrType();

    if (WhoAmI() == Type::reader) {
        m_captureSem = reader_sem;
        m_releaseSem = writer_sem;
    } else {
        m_captureSem = writer_sem;
        m_releaseSem = reader_sem;
    }
}

toSend SharedMemoryWrapper::GetNext() {
    toSend buffer = std::make_unique<SyncBuffer>(m_shmPtr + sizeof(int) + currIdx * sizeof(Buffer), m_captureSem,
                                                 m_releaseSem);
    currIdx = 1 - currIdx;
    return buffer;
}

SharedMemoryWrapper::~SharedMemoryWrapper() {
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

    if (munmap(m_shmPtr, SHM_SIZE) == -1) {
        perror("munmap");
        exit(1);
    }

    if (close(m_shmFd) == -1) {
        perror("close");
        exit(1);
    }
}

Type SharedMemoryWrapper::WhoAmI() const {
    return type;
}

void SharedMemoryWrapper::SetCurrType() {
    sem_wait(m_counterSem);
    int *counter = reinterpret_cast<int *>(m_shmPtr);
    type = static_cast<Type>(*counter % 2);
    ++(*counter);
    sem_post(m_counterSem);
}
