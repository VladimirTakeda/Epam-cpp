#pragma once

#include <fstream>
#include <boost/fiber/buffered_channel.hpp>

#include "ThreadSafeQueue.h"

class Task{
public:
    virtual void Run() = 0;
    virtual ~Task();
};

class ReadTask : public Task {
public:
    explicit ReadTask(std::ifstream &in, ThreadSafeQueue &m_buffers);
    void Run() override;
private:
    std::ifstream &m_in;
    ThreadSafeQueue &m_buffers;
};

class WriteTask : public Task {
public:
    explicit WriteTask(std::ofstream &out, ThreadSafeQueue &m_buffers);
    void Run() override;
private:
    std::ofstream &m_out;
    ThreadSafeQueue &m_buffers;
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