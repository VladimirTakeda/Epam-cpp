#pragma once

#include <memory>
#include <condition_variable>
#include <semaphore.h>

/// A buffer (on shared memory) for processes communication
struct Buffer {
    //because default constructor initilize m_length with zero
    Buffer() {}
    char m_data[1024 * 1024]; // can't initilize, because writer need the data from shared memory
    size_t m_length; // can't initilize, bacause we need the length from shared_memory
};

class CustomDeleter {
public:
    void operator()(Buffer* ptr) const {
        ptr->~Buffer();
    }
};

typedef std::unique_ptr<Buffer, CustomDeleter> BufferPtr;

/// A buffer (on shared memory) wrapper with Posix semaphore (on process memory)
struct SyncBuffer {
    /// @brief wait untill we are able to create a Buffer and create a Buffer
    explicit SyncBuffer(void* sharedMemory, sem_t * capture = nullptr, sem_t * release = nullptr);
    /// @brief notify other process that it may go ahead with writing/reading
    ~SyncBuffer();
    sem_t *m_release;
    BufferPtr buffer;
};

typedef std::unique_ptr<SyncBuffer> toSend;


// I write comments only in English