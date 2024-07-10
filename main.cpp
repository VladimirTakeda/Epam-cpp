#include "dataqueue.h"
#include "sharedmemorymanager.h"
#include "task.h"
#include "threadpool.h"
#include "util.h"

#include <csignal>
#include <fcntl.h>
#include <filesystem>
#include <fstream>
#include <functional>
#include <iostream>
#include <memory>
#include <sys/mman.h>
#include <thread>
#include <unistd.h>

// Task : 1) We need to define who has created shared memory
//        2) We need to syncronize sem_open and ftruncate between 2 processes (we can't invoke ftruncate for one memory at the
//        same time)

// Implementation : 1.1) We need to use O_CREAT | O_EXCL in sem_open to handle EEXIST error
//                  1.2) We need to invoke sem_open 2 times for process with EEXIST error
//                  2.1) We will use named semaphore to syncronize sem_open and ftruncate

// Problems : 1) We have too many if else to hanle in parallel processes
//            2) We have to define who is responsible for semaphore removal (possible the process who has created it)
//            3) We have intinite loop, because we invoke sem_open 2+ times. (in first time we check if it was exist using O_CREAT
//            | O_EXCL, if yes, we invoke
//              sem_open for the second time, but again we have to check who has created the semaptore, current thread or another
//              one, so we need to invoke sem_open with O_CREAT | O_EXCL params and check EEXIST error, if it exist we need to
//              invoke sem_open again and again.

void MeasureTime(const std::function<void()>& func)
{
    auto start = std::chrono::high_resolution_clock::now();
    func();
    auto end                               = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> duration = end - start;
    std::cout << "Queue: " << std::chrono::duration_cast<std::chrono::microseconds>(duration).count() << " microsec" << std::endl;
}

int main(int argc, char* argv[])
{
    if (argc != 4) {
        std::cerr << "wrong arguments count" << std::endl;
        return 1;
    }

    {
        const std::string semPrefix = std::string(argv[3]);

        std::unique_ptr<SharedMemoryManager> sharedObject = std::make_unique<SharedMemoryManager>(semPrefix, argv[3]);
        if (sharedObject->WhoAmI() == Type::none) {
            std::cout << std::this_thread::get_id() << " I don't know who am I" << std::endl;
            return 1;
        }

        std::unique_ptr<Task> IOtask;
        std::unique_ptr<Task> EventReaderTask;
        std::unique_ptr<Task> EventWriterTask;

        SharedQueueBuffer firstBuf(2, sharedObject->GetQueueByIndex(0), semPrefix + '3', semPrefix + '4');
        SharedQueueBuffer secondBuf(2, sharedObject->GetQueueByIndex(1), semPrefix + '5', semPrefix + '6');

        // can't move to inner bacause the tasks are alive till the end of the programm
        std::unique_ptr<DataQueue> fromEventToIO = std::make_unique<DataQueue>(1);
        std::unique_ptr<DataQueue> fromIOToEvent = std::make_unique<DataQueue>(1);

        std::unique_ptr<InterProcessDataQueue> dataQueueFromSharedMemoryToEventReader;

        if (sharedObject->WhoAmI() == Type::reader) {
            std::cout << std::this_thread::get_id() << " I am reader" << std::endl;
            dataQueueFromSharedMemoryToEventReader = std::make_unique<InterProcessDataQueue>(firstBuf, secondBuf);

            fromEventToIO->sendBuffer(sharedObject->GetBufferByIndex(0));
            fromEventToIO->sendBuffer(sharedObject->GetBufferByIndex(1));

            EventReaderTask = CreateEventReaderTask(*sharedObject, *fromEventToIO, *dataQueueFromSharedMemoryToEventReader);
            EventWriterTask = CreateEventWriterTask(*fromIOToEvent, *dataQueueFromSharedMemoryToEventReader);
            IOtask          = CreateReadTask(argv[1], *fromEventToIO, *fromIOToEvent);
        } else if (sharedObject->WhoAmI() == Type::writer) {
            /// need to push 2 free buffers to reader
            std::cout << std::this_thread::get_id() << " I am writer" << std::endl;
            EraseFile(argv[2]);

            dataQueueFromSharedMemoryToEventReader = std::make_unique<InterProcessDataQueue>(secondBuf, firstBuf);

            EventReaderTask = CreateEventReaderTask(*sharedObject, *fromEventToIO, *dataQueueFromSharedMemoryToEventReader);
            EventWriterTask = CreateEventWriterTask(*fromIOToEvent, *dataQueueFromSharedMemoryToEventReader);
            IOtask          = CreateWriteTask(argv[2], *fromEventToIO, *fromIOToEvent);
        }

        MeasureTime([&]() {
            ThreadPool pool(2);
            pool.enqueue(std::move(IOtask));
            pool.enqueue(std::move(EventReaderTask));
            pool.enqueue(std::move(EventWriterTask));
        });
    }
}
