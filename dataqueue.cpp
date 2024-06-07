#include <iostream>

#include "dataqueue.h"

SyncBuffer::SyncBuffer(void *sharedMemory, sem_t *capture, sem_t *release) {
    sem_wait(capture);
    m_release = release;
    buffer = BufferPtr(new(sharedMemory) Buffer());
}

SyncBuffer::~SyncBuffer() {
    sem_post(m_release);
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
