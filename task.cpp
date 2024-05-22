#include "task.h"

Task::~Task() = default;


ReadTask::ReadTask(std::ifstream &in, boost::fibers::buffered_channel<std::vector<char>> &chan) : m_in(in),
                                                                                                  m_chan(chan) {

}

void ReadTask::Run() {
    std::vector<char> buf(BlockSize);

    while (m_in.read(buf.data(), BlockSize) || m_in.gcount() > 0) {
        std::size_t bytesRead = m_in.gcount();
        buf.resize(bytesRead);
        m_chan.push(buf);
    }
    m_chan.close();
}

WriteTask::WriteTask(std::ofstream &out, boost::fibers::buffered_channel<std::vector<char>> &chan) : m_out(out),
                                                                                                     m_chan(chan) {

}

void WriteTask::Run() {
    std::vector<char> buf;
    while (boost::fibers::channel_op_status::success == m_chan.pop(buf)) {
        m_out.write(buf.data(), buf.size());
    }
}