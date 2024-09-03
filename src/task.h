#pragma once

#include "dataqueue/interthreaddataqueue.h"
#include "dataqueue/sharedbuffer.h"

#include <fstream>

class Task {
public:
    /// it will be run it separate thread
    virtual void Run() = 0;
    /// I would like you to stop
    // virtual void Stop() = 0;
    /// return my id
    virtual size_t ID();
    virtual ~Task() = default;

private:
    size_t m_ID = 0;
};

std::unique_ptr<Task> CreateWriteTask(const char* fileName, DataQueue& dataQueueFromEventToIO, DataQueue& dataQueueFromIOToEvent);
std::unique_ptr<Task> CreateReadTask(const char* fileName, DataQueue& dataQueueFromEventToIO, DataQueue& dataQueueFromIOToEvent);