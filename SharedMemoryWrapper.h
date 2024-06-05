#pragma once

#include <fcntl.h>
#include <sys/mman.h>
#include <errno.h>
#include <unistd.h>
#include <semaphore.h>

#include <string>
#include <exception>

#include "dataqueue.h"

#define SHM_SIZE 4096

// how to check if it reader or writer? //int field + one more semaphore?

// how to protect from parallel reading and writing
// how to organize processes syncronization
// sem_open - atomic operation
// sem_writer = sem_open(SEM_NAME_WRITER, O_CREAT, 0666, 1); - last value for closed or opened semaphore
// sem_timedwait for timeout
// what is mac os stack size - 8176 byte
// Natural alignment, for example, on a word boundary at 0x1004. The ARM compiler normally aligns variables and pads structures so that these items are accessed efficiently using LDR and STR instructions.
// Natural alignment is when an object is aligned to its size. For example, a 32-bit integer is naturally aligned when it is 4-byte aligned. For most types on most architectures, natural alignment is the only requirement. For example, on Linux/x86-64, the ABI requires only natural alignment: int is 4-byte aligned, long and pointers are 8-byte aligned.

constexpr std::string_view SEM_NAME_WRITER = "writer_sem";
constexpr std::string_view SEM_NAME_READER = "reader_sem";
constexpr std::string_view SEM_NAME_COUNTER = "counter_sem";

enum class Type : uint8_t {
    reader = 0,
    writer = 1
};

class SharedMemoryWrapper {
public:
    SharedMemoryWrapper() = default;
    explicit SharedMemoryWrapper(char *name) {
        m_shmFd = shm_open(name, O_RDWR, 0666);
        if (m_shmFd < 0) {
            printf("shm_open() failed %s (%d)\n", strerror(errno), errno);
            throw std::bad_alloc();
        }

        m_shmPtr = static_cast<char *>(mmap(0, SHM_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, m_shmFd, 0));
        if (m_shmPtr == MAP_FAILED) {
            perror("mmap");
            throw std::bad_alloc();
        }

        m_counterSem = sem_open(SEM_NAME_COUNTER.data(), O_CREAT, 0666, 1);

        sem_t *writer_sem = sem_open(SEM_NAME_WRITER.data(), O_CREAT, 0666, 0);
        sem_t *reader_sem = sem_open(SEM_NAME_READER.data(), O_CREAT, 0666, 2);

        if (WhoAmI() == Type::reader){
            m_captureSem = reader_sem;
            m_releaseSem = writer_sem;
        }
        else{
            m_captureSem = writer_sem;
            m_releaseSem = reader_sem;
        }
    }

    [[nodiscard]] Type WhoAmI() const{
        bool answer;
        wait(m_counterSem);
        int *counter = reinterpret_cast<int*>(m_shmPtr);
        answer = (*counter % 2);
        ++(*counter);
        sem_post(m_counterSem);
        return static_cast<Type>(answer);
    }

    toSend GetNext(){
        auto buffer = toSend(new(m_shmPtr + sizeof(int) + currIdx * sizeof(Buffer)) Buffer(m_captureSem, m_releaseSem));
        currIdx = 1 - currIdx;
        return buffer;
    }

    ~SharedMemoryWrapper(){
        if (munmap(m_shmPtr, SHM_SIZE) == -1) {
            perror("munmap");
            exit(1);
        }

        if (close(m_shmFd) == -1) {
            perror("close");
            exit(1);
        }
    }

    int currIdx = 0;
    sem_t *m_counterSem = nullptr;
    sem_t *m_captureSem = nullptr;
    sem_t *m_releaseSem = nullptr;
    char* m_shmPtr{};
    int m_shmFd{};
};
