#pragma once

#include <memory>
#include <condition_variable>
#include <semaphore.h>

struct Buffer {
    //because default constructor initilize m_length with zero
    Buffer() {}
    char m_data[1024 * 1024]; // can't initilize, because writer need the data from shared memory
    size_t m_length; // can't initilize, bacause we need the length from shared_memory
};

class CustomDeleter {
public:
    void operator()(Buffer* ptr) const {
    }
};

typedef std::unique_ptr<Buffer, CustomDeleter> BufferPtr;

struct SyncBuffer {
    explicit SyncBuffer(void* sharedMemory, sem_t * capture = nullptr, sem_t * release = nullptr);
    ~SyncBuffer();
    sem_t *m_release;
    BufferPtr buffer;
};

typedef std::unique_ptr<SyncBuffer> toSend;