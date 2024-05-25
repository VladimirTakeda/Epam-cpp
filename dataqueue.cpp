#include "dataqueue.h"

void DataQueue::sendData(toSend &&data){
    {
        std::lock_guard<std::mutex> lock(m_guard);
        m_storage.push(std::move(data));
    }
    m_condVar.notify_one();
}

toSend DataQueue::receiveData() {
    std::unique_lock<std::mutex> lock(m_guard);
    if (m_storage.empty()){
        m_condVar.wait(lock, [&]() {
            return !m_storage.empty();
        });
    }
    auto data = std::move(m_storage.front());
    m_storage.pop();
    return data;
}

//1 (receiveData) WRQueue pick block1 + release cv WRQueue
//2 (sendData) to RWQueue + set cv RWQueue
//3 (receiveData) WRQueue pick block2 + release cv WRQueue

/*int k = 1;
int z = 7;
//timeline
std::atomic_int e = 24; //barrier
double d = 5.;
k += z;*/
