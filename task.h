#pragma once

#include <fstream>
#include <boost/fiber/buffered_channel.hpp>

#include "dataqueue.h"
#include "SharedMemoryWrapper.h"

class Task{
public:
    virtual void Run() = 0;
    virtual ~Task();
};

/// ifstream Reader
class ReadTask : public Task {
public:
    explicit ReadTask(std::ifstream &in, SharedMemoryWrapper &sharedMemoryWrapper);
    /// @brief takes free buffer, reads data, pass to Writer
    void Run() override;
private:
    std::ifstream &m_in;
    SharedMemoryWrapper &m_sharedMemoryWrapper;
};

/// ofstream Writer
class WriteTask : public Task {
public:
    explicit WriteTask(std::ofstream &out, SharedMemoryWrapper &sharedMemoryWrapper);
    /// @brief takes free buffer, writes data, pass to Reader
    void Run() override;
private:
    std::ofstream &m_out;
    SharedMemoryWrapper &m_sharedMemoryWrapper;
};