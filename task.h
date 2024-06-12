#pragma once

#include <fstream>

#include "dataqueue.h"

class Task{
public:
    virtual void Run() = 0;
    virtual ~Task();
};

/// ifstream Reader
class ReadTask : public Task {
public:
    explicit ReadTask(std::ifstream &in, DataQueue &dataQueueFromReaderToWriter, DataQueue &dataQueueFromWriterToReader);
    /// @brief takes free buffer, reads data, pass to Writer
    void Run() override;
private:
    std::ifstream &m_in;
    DataQueue &m_dataQueueFromReaderToWriter;
    DataQueue &m_dataQueueFromWriterToReader;
};

/// ofstream Writer
class WriteTask : public Task {
public:
    explicit WriteTask(std::ofstream &out, DataQueue &dataQueueFromReaderToWriter, DataQueue &dataQueueFromWriterToReader);
    /// @brief takes free buffer, writes data, pass to Reader
    void Run() override;
private:
    std::ofstream &m_out;
    DataQueue &m_dataQueueFromReaderToWriter;
    DataQueue &m_dataQueueFromWriterToReader;
};