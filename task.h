#pragma once

#include <fstream>
#include <boost/fiber/buffered_channel.hpp>

constexpr int BlockSize = 1024 * 1024;

class Task{
public:
    virtual void Run() = 0;
    virtual ~Task();
};

class ReadTask : public Task {
public:
    explicit ReadTask(std::ifstream &in, boost::fibers::buffered_channel<std::vector<char>> &chan);
    void Run() override ;
private:
    std::ifstream &m_in;
    boost::fibers::buffered_channel<std::vector<char>> &m_chan;
};

class WriteTask : public Task {
public:
    explicit WriteTask(std::ofstream &out, boost::fibers::buffered_channel<std::vector<char>> &chan);
    void Run() override;
private:
    std::ofstream &m_out;
    boost::fibers::buffered_channel<std::vector<char>> &m_chan;
};