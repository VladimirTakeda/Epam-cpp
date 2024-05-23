#include "task.h"

Task::~Task() = default;


ReadTask::ReadTask(std::ifstream &in, ThreadSafeQueue &buffers) : m_in(in),
                                                                  m_buffers(buffers) {

}

void ReadTask::Run() {
    while (m_buffers.Read(m_in)) {
    }
    m_buffers.isDone = true;
    std::cout << "endRead\n";
}

WriteTask::WriteTask(std::ofstream &out, ThreadSafeQueue &buffers) : m_out(out),
                                                                       m_buffers(buffers) {

}

void WriteTask::Run() {
    while (m_buffers.Write(m_out)) {
    }
    std::cout << "endWrite\n";
}


ReadTaskBoost::ReadTaskBoost(std::ifstream &in, boost::fibers::buffered_channel<std::vector<char>> &chan) : m_in(in),
                                                                                                  m_chan(chan) {

}

void ReadTaskBoost::Run() {
    std::vector<char> buf(BlockSize);

    while (m_in.read(buf.data(), BlockSize) || m_in.gcount() > 0) {
        std::size_t bytesRead = m_in.gcount();
        buf.resize(bytesRead);
        m_chan.push(buf);
    }
    std::cout << "endReadBoost\n";
    m_chan.close();
}

WriteTaskBoost::WriteTaskBoost(std::ofstream &out, boost::fibers::buffered_channel<std::vector<char>> &chan) : m_out(out),
                                                                                                     m_chan(chan) {

}

void WriteTaskBoost::Run() {
    std::vector<char> buf;
    while (boost::fibers::channel_op_status::success == m_chan.pop(buf)) {
        m_out.write(buf.data(), buf.size());
    }
    std::cout << "endWriteBoost\n";
}