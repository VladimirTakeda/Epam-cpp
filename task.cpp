#include <iostream>

#include "task.h"

constexpr int BlockSize = 1024 * 1024;
constexpr size_t Empty = 0;

Task::~Task() = default;


ReadTask::ReadTask(std::ifstream &in, SharedMemoryWrapper &sharedMemoryWrapper) : m_in(in),
                                                                  m_sharedMemoryWrapper(sharedMemoryWrapper){

}

void ReadTask::Run() {
    auto toSend = m_sharedMemoryWrapper.GetNext();
    while (m_in.read(toSend->buffer->m_data, BlockSize) || m_in.gcount() > 0) {
        std::cout << "was startRead... " << m_in.gcount() << "\n";
        std::size_t bytesRead = m_in.gcount();
        toSend->buffer->m_length = bytesRead;
        toSend.reset();
        toSend = m_sharedMemoryWrapper.GetNext();
        std::cout << "loopRead\n";
    }
    std::cout << "was endRead... " << m_in.gcount() << "\n";
    toSend->buffer->m_length = Empty;
    std::cout << "endRead\n";
}

WriteTask::WriteTask(std::ofstream &out, SharedMemoryWrapper &sharedMemoryWrapper) : m_out(out),
                                                                                     m_sharedMemoryWrapper(sharedMemoryWrapper){

}

void WriteTask::Run() {
    auto toSend = m_sharedMemoryWrapper.GetNext();
    while (toSend->buffer->m_length) {
        m_out.write(toSend->buffer->m_data, toSend->buffer->m_length);
        std::cout << "was ...startWrite\n";
        toSend.reset();
        toSend = m_sharedMemoryWrapper.GetNext();
        std::cout << "loopWrite\n";
    }
    std::cout << "endWrite\n";
}