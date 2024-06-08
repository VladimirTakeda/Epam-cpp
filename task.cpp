#include "task.h"

constexpr int BlockSize = 1024 * 1024;
constexpr size_t Empty = 0;

Task::~Task() = default;


ReadTask::ReadTask(std::ifstream &in, SharedMemoryManager &sharedMemoryWrapper) : m_in(in),
                                                                  m_sharedMemoryWrapper(sharedMemoryWrapper){

}

bool ReadTask::Run() {
    auto toSend = m_sharedMemoryWrapper.GetNext();
    while (toSend && (m_in.read(toSend->buffer->m_data, BlockSize) || m_in.gcount() > 0)) {
        std::size_t bytesRead = m_in.gcount();
        toSend->buffer->m_length = bytesRead;
        toSend.reset();
        toSend = m_sharedMemoryWrapper.GetNext();
    }
    if (toSend) {
        toSend->buffer->m_length = Empty;
        return true;
    }
    return false;
}

WriteTask::WriteTask(std::ofstream &out, SharedMemoryManager &sharedMemoryWrapper) : m_out(out),
                                                                                     m_sharedMemoryWrapper(sharedMemoryWrapper){

}

bool WriteTask::Run() {
    auto toSend = m_sharedMemoryWrapper.GetNext();
    while (toSend && toSend->buffer->m_length) {
        m_out.write(toSend->buffer->m_data, toSend->buffer->m_length);
        toSend.reset();
        toSend = m_sharedMemoryWrapper.GetNext();
    }
    if (toSend) {
        return true;
    }
    return false;
}