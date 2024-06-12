#pragma once

#include <string>

#include "dataqueue.h"

constexpr int SHM_SIZE = 2 * 1024 * 1024 + 100;

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
constexpr std::string_view FROM_READER_TO_WRITER_PIPE_NAME = "/dev/shm/from_reader_to_writer_pipe";
constexpr std::string_view FROM_WRITER_TO_READER_PIPE_NAME = "/dev/shm/from_writer_to_reader_pipe";

enum class Type : uint8_t {
    reader = 0,
    writer = 1,
    none = 2
};

/// A class to manipulate shared memory object
class SharedMemoryManager {
public:
    SharedMemoryManager() = default;
    /// @brief opens semaphores, link shared memory in this process, set the process type (reader/writer)
    explicit SharedMemoryManager(const char *SharedObjName);
    /// @brief return next data buffer from shared memory
    toSend GetNext();
    /// @brief decrement the number of active processes, unlink shared memory object and close semaphore if nesessary
    ~SharedMemoryManager();
    /// @brief return the type of current process (reader/writer)
    [[nodiscard]] Type WhoAmI() const;

    int GetFromReaderToWriterPipe() const;
    int GetFromWriterToReaderPipe() const;
    char *GetSharedMemoryPtr() const;

private:
    /// @brief define who the currect proccess is (reader/writer)
    void SetCurrType();

private:
    Type type = Type::none;
    int currIdx = 0;
    sem_t *m_counterSem = nullptr;
    sem_t *m_captureSem = nullptr;
    sem_t *m_releaseSem = nullptr;
    char* m_shmPtr{};
    int m_shmFd{};
    int m_namedPipeFromReaderToWriter{};
    int m_namedPipeFromWriterToReader{};
};
