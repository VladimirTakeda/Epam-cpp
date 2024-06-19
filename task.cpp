#include "task.h"
#include "util.h"

constexpr int BlockSize = 1024 * 1024;
constexpr size_t Empty = 0;

Task::~Task() = default;


ReadTask::ReadTask(char *fileName, SharedMemoryManager &sharedMemoryWrapper, TimedDataQueue &dataQueueFromIOToNotifier, InterProcessDataQueue &dataQueueFromNotifierToIO)
: m_in(fileName), m_sharedMemoryWrapper(sharedMemoryWrapper), m_dataQueueFromIOToNotifier(dataQueueFromIOToNotifier), m_dataQueueFromNotifierToIO(dataQueueFromNotifierToIO){

}

void ReadTask::Run() {
    size_t index = m_dataQueueFromNotifierToIO.receiveData();
    auto toSend = m_sharedMemoryWrapper.GetBufferByIndex(index);
    while (m_in.read(toSend->m_data, BlockSize) || m_in.gcount() > 0) {
        std::size_t bytesRead = m_in.gcount();
        toSend->m_length = bytesRead;
        m_dataQueueFromIOToNotifier.sendIndex(index);
        toSend = m_sharedMemoryWrapper.GetBufferByIndex(m_dataQueueFromNotifierToIO.receiveData());
    }
}

WriteTask::WriteTask(char *fileName, SharedMemoryManager &sharedMemoryWrapper, TimedDataQueue &dataQueueFromIOToNotifier, InterProcessDataQueue &dataQueueFromNotifierToIO)
: m_out(fileName), m_sharedMemoryWrapper(sharedMemoryWrapper), m_dataQueueFromIOToNotifier(dataQueueFromIOToNotifier), m_dataQueueFromNotifierToIO(dataQueueFromNotifierToIO){

}

void WriteTask::Run() {
    auto toSend = m_sharedMemoryWrapper.GetNext();
    while (toSend->m_length) {
        m_out.write(toSend->m_data, toSend->m_length);
        toSend.reset();
        toSend = m_sharedMemoryWrapper.GetNext();
    }
}

PeriodicNotifierTask::PeriodicNotifierTask(size_t timeout, TimedDataQueue &dataQueueFromIOToNotifier, InterProcessDataQueue &dataQueueFromNotifierToIO)
: m_dataQueueFromIOToNotifier(dataQueueFromIOToNotifier), m_dataQueueFromNotifierToIO(dataQueueFromNotifierToIO) {
}

void PeriodicNotifierTask::Run() {

}