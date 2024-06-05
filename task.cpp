#include "task.h"

constexpr int BlockSize = 1024 * 1024;
constexpr size_t Empty = 0;

Task::~Task() = default;


ReadTask::ReadTask(std::ifstream &in, SharedMemoryWrapper &sharedMemoryWrapper) : m_in(in),
                                                                  m_sharedMemoryWrapper(sharedMemoryWrapper){

}

void ReadTask::Run() {
    auto toSend = m_sharedMemoryWrapper.GetNext();
    while (m_in.read(toSend->m_data, BlockSize) || m_in.gcount() > 0) {
        std::size_t bytesRead = m_in.gcount();
        toSend->m_length = bytesRead;
        toSend = m_sharedMemoryWrapper.GetNext();
    }
    toSend->m_length = Empty;
    std::cout << "endRead\n";
}

WriteTask::WriteTask(std::ofstream &out, SharedMemoryWrapper &sharedMemoryWrapper) : m_out(out),
                                                                                     m_sharedMemoryWrapper(sharedMemoryWrapper){

}

void WriteTask::Run() {
    auto toSend = m_sharedMemoryWrapper.GetNext();
    while (toSend->m_length) {
        m_out.write(toSend->m_data, toSend->m_length);
        toSend = m_sharedMemoryWrapper.GetNext();
    }
    std::cout << "endWrite\n";
}