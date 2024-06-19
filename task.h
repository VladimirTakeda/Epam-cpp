#pragma once

#include <fstream>

#include "dataqueue.h"
#include "sharedmemorymanager.h"

class Task{
public:
    virtual void Run() = 0;
    virtual ~Task();
};

/// ifstream Reader
class ReadTask final : public Task {
public:
    explicit ReadTask(char* fileName, SharedMemoryManager &sharedMemoryWrapper, TimedDataQueue &dataQueueFromIOToNotifier, InterProcessDataQueue &dataQueueFromNotifierToIO);
    /// @brief takes free buffer index from other process, if it's valid do read task and push the index to Notifier via TimedDataQueue
    void Run() override;
private:
    std::ifstream m_in;
    SharedMemoryManager &m_sharedMemoryWrapper;

    TimedDataQueue &m_dataQueueFromIOToNotifier;
    InterProcessDataQueue &m_dataQueueFromNotifierToIO;
};

/// ofstream Writer
class WriteTask final : public Task {
public:
    explicit WriteTask(char* fileName, SharedMemoryManager &sharedMemoryWrapper, TimedDataQueue &dataQueueFromIOToNotifier, InterProcessDataQueue &dataQueueFromNotifierToIO);
    /// @brief takes free buffer index from other process, if it's valid do write task and push the index to Notifier via TimedDataQueue
    void Run() override;
private:
    /// @brief erase destination in case of error
    void CleanUp();
private:
    std::ofstream m_out;
    SharedMemoryManager &m_sharedMemoryWrapper;

    TimedDataQueue &m_dataQueueFromIOToNotifier;
    InterProcessDataQueue &m_dataQueueFromNotifierToIO;
};

class PeriodicNotifierTask final: public Task {
public:
    explicit PeriodicNotifierTask(size_t timeout, TimedDataQueue &dataQueueFromIOToNotifier, InterProcessDataQueue &dataQueueFromNotifierToIO);
    /// @brief notify other thread that he can do the job or ping him
    void Run() override;
private:
    TimedDataQueue &m_dataQueueFromIOToNotifier;
    InterProcessDataQueue &m_dataQueueFromNotifierToIO;
};