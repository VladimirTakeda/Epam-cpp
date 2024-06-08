#include <iostream>

#include "dataqueue.h"

SyncBuffer::SyncBuffer(void *sharedMemory, sem_t *capture, sem_t *release) {
    timespec ts{};
    clock_gettime(CLOCK_REALTIME, &ts);
    ts.tv_sec += 5;  // Установим тайм-аут на 5 секунд вперед

    sem_timedwait(capture, &ts);
    if (errno == ETIMEDOUT)
        throw std::runtime_error("semaphore timeout occurred!");

    m_release = release;
    buffer = BufferPtr(new(sharedMemory) Buffer());
}

SyncBuffer::~SyncBuffer() {
    sem_post(m_release);
}
