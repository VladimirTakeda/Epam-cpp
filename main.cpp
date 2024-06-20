#include <fstream>
#include <iostream>
#include <memory>
#include <filesystem>
#include <csignal>
#include <thread>
#include <functional>

#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>

#include "task.h"
#include "dataqueue.h"
#include "sharedmemorymanager.h"
#include "threadpool.h"
#include "util.h"

// Task : 1) We need to define who has created shared memory
//        2) We need to syncronize sem_open and ftruncate between 2 processes (we can't invoke ftruncate for one memory at the same time)

// Implementation : 1.1) We need to use O_CREAT | O_EXCL in sem_open to handle EEXIST error
//                  1.2) We need to invoke sem_open 2 times for process with EEXIST error
//                  2.1) We will use named semaphore to syncronize sem_open and ftruncate

// Problems : 1) We have too many if else to hanle in parallel processes
//            2) We have to define who is responsible for semaphore removal (possible the process who has created it)
//            3) We have intinite loop, because we invoke sem_open 2+ times. (in first time we check if it was exist using O_CREAT | O_EXCL, if yes, we invoke
//              sem_open for the second time, but again we have to check who has created the semaptore, current thread or another one, so we need to invoke
//              sem_open with O_CREAT | O_EXCL params and check EEXIST error, if it exist we need to invoke sem_open again and again.

void MeasureTime(const std::function<void()>& func) {
    auto start = std::chrono::high_resolution_clock::now();
    func();
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> duration = end - start;
    std::cout << "Queue: " << std::chrono::duration_cast<std::chrono::microseconds>(duration).count() << " microsec" << std::endl;
}

int main(int argc, char *argv[]){
    if (argc != 4){
        std::cerr << "wrong arguments count" << std::endl;
        return 1;
    }

    {
        const std::string semPrefix = std::string(argv[3]);

        std::unique_ptr<SharedMemoryManager> sharedObject = std::make_unique<SharedMemoryManager>(semPrefix, argv[3]);
        if (sharedObject->WhoAmI() == Type::none) {
            std::cout << std::this_thread::get_id() << " I don't know who am I" << std::endl;
            return 0;
        }

        std::unique_ptr<Task> IOtask;
        std::unique_ptr<Task> Notifier;
        std::unique_ptr<Task> HealthChecker;
        std::unique_ptr<Task> HealthPing;

        SharedQueueBuffer firstBuf(sharedObject->GetQueueByIndex(0), semPrefix + '3', semPrefix + '4');
        SharedQueueBuffer secondBuf(sharedObject->GetQueueByIndex(0), semPrefix + '5', semPrefix + '6');

        InterProcessDataQueue dataQueueFromReaderToWriter(firstBuf, secondBuf);
        InterProcessDataQueue dataQueueFromWriterToReader(secondBuf, firstBuf);

        //can't move to inner bacause the tasks are alive till the end of the programm
        std::unique_ptr<DataQueue> fromNotifierToReader;
        std::unique_ptr<DataQueue> fromNotifierToWriter;

        if (sharedObject->WhoAmI() == Type::reader){
            std::cout << std::this_thread::get_id() << " I am reader" << std::endl;
            fromNotifierToReader = std::make_unique<DataQueue>();

            fromNotifierToReader->sendIndex(0);
            fromNotifierToReader->sendIndex(1);

            HealthChecker = std::make_unique<HealthCheckerTask>(2, semPrefix + '7');
            HealthPing = std::make_unique<HealthPingTask>(1, semPrefix + '8');
            Notifier = std::make_unique<NotifierTask>(*fromNotifierToReader, dataQueueFromReaderToWriter);
            IOtask = std::make_unique<ReadTask>(argv[1], *sharedObject, *fromNotifierToReader, dataQueueFromWriterToReader);
        }
        else{
            /// need to push 2 free buffers to reader
            std::cout << std::this_thread::get_id() << " I am writer" << std::endl;
            EraseFile(argv[2]);
            fromNotifierToWriter = std::make_unique<DataQueue>();

            HealthChecker = std::make_unique<HealthCheckerTask>(2, semPrefix + '8');
            HealthPing = std::make_unique<HealthPingTask>(1, semPrefix + '7');
            Notifier = std::make_unique<NotifierTask>(*fromNotifierToWriter, dataQueueFromWriterToReader);
            IOtask = std::make_unique<WriteTask>(argv[2], *sharedObject, *fromNotifierToWriter, dataQueueFromReaderToWriter);
        }

        MeasureTime([&]() {
            ThreadPool pool(3);
            pool.enqueue(std::move(IOtask));
            pool.enqueue(std::move(Notifier));
            pool.enqueue(std::move(HealthChecker));
            pool.enqueue(std::move(HealthPing));
        });
    }
}
