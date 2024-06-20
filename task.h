#pragma once

#include <fstream>

#include "dataqueue.h"
#include "sharedmemorymanager.h"

class Task{
public:
    virtual void Run(std::stop_source stopSource) = 0;
    virtual ~Task();
};

/// ifstream Reader
///
/// reader can read 2 buffers from the beginning independently from writer
class ReadTask final : public Task {
public:
    explicit ReadTask(const char* fileName, SharedMemoryManager &sharedMemoryWrapper, DataQueue &dataQueueFromNotifierToIO, InterProcessDataQueue& dataQueueFromIOToNotifier);
    /// @brief takes free buffer index from other process, if it's valid do read task and push the index to Notifier via TimedDataQueue
    void Run(std::stop_source stopSource) override;
private:
    std::ifstream m_in;
    SharedMemoryManager &m_sharedMemoryWrapper;

    DataQueue &m_dataQueueFromNotifierToIO;
    InterProcessDataQueue &m_dataQueueFromIOToNotifier;
};

/// ofstream Writer
class WriteTask final : public Task {
public:
    explicit WriteTask(const char* fileName, SharedMemoryManager &sharedMemoryWrapper, DataQueue &dataQueueFromNotifierToIO, InterProcessDataQueue& dataQueueFromIOToNotifier);
    /// @brief takes free buffer index from other process, if it's valid do write task and push the index to Notifier via TimedDataQueue
    void Run(std::stop_source stopSource) override;
private:
    /// @brief erase destination in case of error
    /// don't create method for one invoke
    void CleanUp();
private:
    std::ofstream m_out;
    SharedMemoryManager &m_sharedMemoryWrapper;

    DataQueue &m_dataQueueFromNotifierToIO;
    InterProcessDataQueue &m_dataQueueFromIOToNotifier;
};

/// Accumulate all the messages from another process to DataQueue
class NotifierTask final: public Task {
public:
    explicit NotifierTask(DataQueue &dataQueueFromNotifierToIO, InterProcessDataQueue& dataQueueFromIOToNotifier);
    /// @brief grab the data from message queue and puts it to IOQueue
    void Run(std::stop_source stopSource) override;
private:
    DataQueue &m_dataQueueFromNotifierToIO;
    InterProcessDataQueue &m_dataQueueFromIOToNotifier;
};

/// Health Indicator for another process
class HealthPingTask final: public Task {
public:
    explicit HealthPingTask(uint32_t timeoutSec, const std::string& pingSemName);
    /// @brief notify other process that I am alive
    void Run(std::stop_source stopSource) override;
private:
    uint32_t m_timeoutSec;
    SemWrapper m_pingSem;
};

/// Health Checker for another process
class HealthCheckerTask final: public Task {
public:
    explicit HealthCheckerTask(uint32_t timeoutSec, const std::string& pingSemName);
    /// @brief check that other process is alive
    void Run(std::stop_source stopSource) override;
private:
    uint32_t m_timeoutSec;
    SemWrapper m_pingSem;
};