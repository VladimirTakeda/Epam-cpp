#include <cstring>
#include <iostream>

#include <fcntl.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <semaphore.h>

#include "sharedmemorymanager.h"

SharedMemoryManager::SharedMemoryManager(const char *SharedObjName) {
    m_shmFd = shm_open(SharedObjName, O_RDWR, 0666);
    if (m_shmFd < 0) {
        throw std::bad_alloc();
    }

    m_shmPtr = static_cast<char *>(mmap(0, SHM_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, m_shmFd, 0));
    if (m_shmPtr == MAP_FAILED) {
        throw std::bad_alloc();
    }

    m_counterSem = sem_open(SEM_NAME_COUNTER.data(), O_CREAT, S_IRUSR | S_IWUSR, 1);

    if (m_counterSem == SEM_FAILED) {
        throw std::logic_error("failed to open semaphore");
    }

    if (mkfifo(FROM_READER_TO_WRITER_PIPE_NAME.data(), 0666) == -1 && errno != EEXIST) {
        throw std::logic_error("Failed to create named pipe");
    }

    if (mkfifo(FROM_WRITER_TO_READER_PIPE_NAME.data(), 0666) == -1 && errno != EEXIST) {
        throw std::logic_error("Failed to create named pipe");
    }

    SetCurrType();

    if (WhoAmI() == Type::reader) {
        m_namedPipeFromReaderToWriter = open(FROM_READER_TO_WRITER_PIPE_NAME.data(), O_WRONLY);
        if (m_namedPipeFromReaderToWriter == -1) {
            throw std::logic_error("failed to open pipe");
        }

        m_namedPipeFromWriterToReader = open(FROM_WRITER_TO_READER_PIPE_NAME.data(), O_RDONLY);
        if (m_namedPipeFromWriterToReader == -1) {
            throw std::logic_error("failed to open pipe");
        }

    } else {
        m_namedPipeFromReaderToWriter = open(FROM_READER_TO_WRITER_PIPE_NAME.data(), O_RDONLY);
        if (m_namedPipeFromReaderToWriter == -1) {
            throw std::logic_error("failed to open pipe");
        }

        m_namedPipeFromWriterToReader = open(FROM_WRITER_TO_READER_PIPE_NAME.data(), O_WRONLY);
        if (m_namedPipeFromWriterToReader == -1) {
            throw std::logic_error("failed to open pipe");
        }
    }
}

toSend SharedMemoryManager::GetNext() {
    auto buffer = toSend(new(m_shmPtr + sizeof(int) + currIdx * sizeof(Buffer)) Buffer());
    currIdx = 1 - currIdx;
    return buffer;
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
        sem_close(m_counterSem);

        sem_unlink(SEM_NAME_COUNTER.data());

        close(m_namedPipeFromWriterToReader);
        close(m_namedPipeFromReaderToWriter);
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

int SharedMemoryManager::GetFromReaderToWriterPipe() const {
    return m_namedPipeFromReaderToWriter;
}

int SharedMemoryManager::GetFromWriterToReaderPipe() const {
    return m_namedPipeFromWriterToReader;
}

char* SharedMemoryManager::GetSharedMemoryPtr() const {
    return m_shmPtr;
}

void SharedMemoryManager::SetCurrType() {
    sem_wait(m_counterSem);
    int *counter = reinterpret_cast<int *>(m_shmPtr);
    type = static_cast<Type>(*counter % 2);
    ++(*counter);
    sem_post(m_counterSem);
}
