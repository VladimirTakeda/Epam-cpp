#pragma once

#include <fstream>
#include <boost/fiber/buffered_channel.hpp>

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

class ReadTaskBoost : public Task {
public:
    explicit ReadTaskBoost(std::ifstream &in, boost::fibers::buffered_channel<std::vector<char>> &chan);
    void Run() override ;
private:
    std::ifstream &m_in;
    boost::fibers::buffered_channel<std::vector<char>> &m_chan;
};

class WriteTaskBoost : public Task {
public:
    explicit WriteTaskBoost(std::ofstream &out, boost::fibers::buffered_channel<std::vector<char>> &chan);
    void Run() override;
private:
    std::ofstream &m_out;
    boost::fibers::buffered_channel<std::vector<char>> &m_chan;
};