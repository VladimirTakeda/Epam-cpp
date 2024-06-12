#include "dataqueue.h"

#include <iostream>
#include <thread>

DataQueue::DataQueue(int pipeId, char *shared_memory) : m_pipeId(pipeId), m_shared_memory(shared_memory){

}

void DataQueue::sendData(toSend &&data){
    Buffer *ptr = data.get();
    size_t diff = reinterpret_cast<char *>(ptr) - m_shared_memory;
    if (int size = write(m_pipeId, &diff, sizeof(diff)); size != sizeof(diff)) {
        throw std::logic_error("failed to write pipe");
    }
}

toSend DataQueue::receiveData() {
    size_t diff = 0;
    if (int size = read(m_pipeId, &diff, sizeof(diff)); size != sizeof(diff)) {
        throw std::logic_error("failed to read pipe");
    }
    return toSend(reinterpret_cast<Buffer *>(m_shared_memory + diff));
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
