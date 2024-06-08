#pragma once

#include <fstream>

#include "dataqueue.h"
#include "sharedmemorymanager.h"

class Task{
public:
    virtual bool Run() = 0;
    virtual ~Task();
};

/// ifstream Reader
class ReadTask : public Task {
public:
    explicit ReadTask(std::ifstream &in, SharedMemoryManager &sharedMemoryWrapper);
    /// @brief takes free buffer, reads data, pass to Writer
    bool Run() override;
private:
    std::ifstream &m_in;
    SharedMemoryManager &m_sharedMemoryWrapper;
};

/// ofstream Writer
class WriteTask : public Task {
public:
    explicit WriteTask(std::ofstream &out, SharedMemoryManager &sharedMemoryWrapper);
    /// @brief takes free buffer, writes data, pass to Reader
    bool Run() override;
private:
    std::ofstream &m_out;
    SharedMemoryManager &m_sharedMemoryWrapper;
};