#include <iostream>
#include <thread>
#include <stdexcept>

#include "task.h"

constexpr int BlockSize = 1024 * 1024;
constexpr size_t Empty = 0;

Task::~Task() = default;


ReadTask::ReadTask(const char * const fileName, SharedMemoryManager &sharedMemoryWrapper, DataQueue &dataQueueFromNotifierToIO, InterProcessDataQueue& dataQueueFromIOToNotifier)
: m_in(fileName), m_sharedMemoryWrapper(sharedMemoryWrapper), m_dataQueueFromIOToNotifier(dataQueueFromIOToNotifier), m_dataQueueFromNotifierToIO(dataQueueFromNotifierToIO){

}

void ReadTask::Run(std::stop_source stopSource) {
    std::cout << std::this_thread::get_id() << " start reading " << std::endl;
    uint32_t index = m_dataQueueFromNotifierToIO.receiveIndex();
    auto toSend = m_sharedMemoryWrapper.GetBufferByIndex(index);
    std::cout << std::this_thread::get_id() << " start reading got buffer " << index << std::endl;
    while (m_in.read(toSend->m_data, BlockSize) || m_in.gcount() > 0) {
        std::cout << std::this_thread::get_id() << " reader try to get token " << std::endl;
        std::stop_token stoken = stopSource.get_token();
        if (stoken.stop_requested())
            break;
        std::cout << std::this_thread::get_id() << " got token " << std::endl;
        std::size_t bytesRead = m_in.gcount();
        std::cout << std::this_thread::get_id() << " butesRead "  << bytesRead << std::endl;
        toSend->m_length = bytesRead;
        m_dataQueueFromIOToNotifier.sendData(index);
        index = m_dataQueueFromNotifierToIO.receiveIndex();
        toSend = m_sharedMemoryWrapper.GetBufferByIndex(index);
        std::cout << std::this_thread::get_id() << " loop reading "  << index << std::endl;
    }
    // TODO: remove this hack for stop NotifierTask
    m_dataQueueFromIOToNotifier.sendData(index);
    stopSource.request_stop();
    std::cout << std::this_thread::get_id() << " end reading "  << index << std::endl;
}

WriteTask::WriteTask(const char * const fileName, SharedMemoryManager &sharedMemoryWrapper, DataQueue &dataQueueFromNotifierToIO, InterProcessDataQueue& dataQueueFromIOToNotifier)
: m_out(fileName), m_sharedMemoryWrapper(sharedMemoryWrapper), m_dataQueueFromIOToNotifier(dataQueueFromIOToNotifier), m_dataQueueFromNotifierToIO(dataQueueFromNotifierToIO){

}

void WriteTask::Run(std::stop_source stopSource) {
    std::cout << std::this_thread::get_id() << " start writing " << std::endl;
    uint32_t index = m_dataQueueFromNotifierToIO.receiveIndex();
    auto toSend = m_sharedMemoryWrapper.GetBufferByIndex(index);
    std::cout << std::this_thread::get_id() << " start writing got buffer " << index << std::endl;
    while (toSend->m_length) {
        std::cout << std::this_thread::get_id() << " writer try to get token " << std::endl;
        std::stop_token stoken = stopSource.get_token();
        if (stoken.stop_requested())
            break;
        m_out.write(toSend->m_data, toSend->m_length);
        m_dataQueueFromIOToNotifier.sendData(index);
        toSend = m_sharedMemoryWrapper.GetBufferByIndex(m_dataQueueFromNotifierToIO.receiveIndex());
        std::cout << std::this_thread::get_id() << " loop writing " << index << std::endl;
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    stopSource.request_stop();
    std::cout << std::this_thread::get_id() << " end writing "  << index << std::endl;
}

NotifierTask::NotifierTask(DataQueue &dataQueueFromNotifierToIO, InterProcessDataQueue& dataQueueFromIOToNotifier)
: m_dataQueueFromNotifierToIO(dataQueueFromNotifierToIO), m_dataQueueFromIOToNotifier(dataQueueFromIOToNotifier) {
}

void NotifierTask::Run(std::stop_source stopSource) {
    uint32_t toSend = m_dataQueueFromIOToNotifier.receiveData();
    std::cout << std::this_thread::get_id() << " send index before loop " << toSend << std::endl;
    while (true) {
        std::cout << "we got a new index " << toSend << std::endl;
        m_dataQueueFromNotifierToIO.sendIndex(toSend);

        std::stop_token stoken = stopSource.get_token();
        if (stoken.stop_requested())
            break;

        // we need to stop wait here
        toSend = m_dataQueueFromIOToNotifier.receiveData();
        std::cout << std::this_thread::get_id() << " send index inside loop " << toSend << std::endl;
    }
    std::cout << std::this_thread::get_id() << " end Notifier "  << std::endl;
}

HealthPingTask::HealthPingTask(uint32_t timeoutSec, const std::string& pingSemName) : m_timeoutSec(timeoutSec), m_pingSem(pingSemName, 0){

}
void HealthPingTask::Run(std::stop_source stopSource) {
    while (true) {
        std::stop_token stoken = stopSource.get_token();
        if (stoken.stop_requested())
            break;
        sem_post(m_pingSem.Get());
        std::this_thread::sleep_for(std::chrono::seconds(m_timeoutSec));
    }
    std::cout << std::this_thread::get_id() << " end HealthPingTask "  << std::endl;
}

HealthCheckerTask::HealthCheckerTask(uint32_t timeoutSec, const std::string& pingSemName) :  m_timeoutSec(timeoutSec), m_pingSem(pingSemName, 0) {

}

void HealthCheckerTask::Run(std::stop_source stopSource) {
    while (true) {
        std::stop_token stoken = stopSource.get_token();
        if (stoken.stop_requested())
            break;
        timespec ts{};
        if (clock_gettime(CLOCK_REALTIME, &ts) == -1){
            throw std::logic_error(std::string("Failed to set up timer"));
        }

        ts.tv_sec += m_timeoutSec;

        auto s = sem_timedwait(m_pingSem.Get(), &ts);

        if (s == -1)
        {
            std::cout << std::this_thread::get_id() << " HealthCheckerTask timeout "  << std::endl;
            if (errno == ETIMEDOUT)
                stopSource.request_stop();
            stopSource.request_stop();
        }
    }
    std::cout << std::this_thread::get_id() << " end HealthCheckerTask "  << std::endl;
}