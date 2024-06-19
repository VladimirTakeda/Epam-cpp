#include <fstream>
#include <iostream>
#include <memory>
#include <filesystem>
#include <csignal>
#include <thread>

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
        const std::string semPrefix = std::string(argv[1]) + argv[2] + argv[3];

        std::unique_ptr<SharedMemoryManager> sharedObject = std::make_unique<SharedMemoryManager>(semPrefix, argv[3]);

        std::unique_ptr<Task> IOtask;
        std::unique_ptr<Task> Notifier;

        InterProcessDataQueue dataQueueFromReaderToWriter(1, sharedObject->GetQueueByIndex(1), sizeof(uint32_t),
            sharedObject->GetQueueByIndex(0), sizeof(uint32_t),
            semPrefix + '5', semPrefix + '6',semPrefix + '3', semPrefix + '4');
        InterProcessDataQueue dataQueueFromWriterToReader(2 , sharedObject->GetQueueByIndex(0), sizeof(uint32_t),
            sharedObject->GetQueueByIndex(1), sizeof(uint32_t),
            semPrefix + '3', semPrefix + '4',semPrefix + '5', semPrefix + '6');

        if (sharedObject->WhoAmI() == Type::reader){
            std::cout << std::this_thread::get_id() << " I am reader" << std::endl;
            TimedDataQueue fromReaderToNofirier(1);
            Notifier = std::make_unique<PeriodicNotifierTask>(1, fromReaderToNofirier, dataQueueFromReaderToWriter);
            IOtask = std::make_unique<ReadTask>(argv[1], *sharedObject, fromReaderToNofirier, dataQueueFromWriterToReader);
        }
        else{
            std::cout << std::this_thread::get_id() << " I am writer" << std::endl;
            EraseFile(argv[2]);
            TimedDataQueue fromWriterToNofirier(2);
            Notifier = std::make_unique<PeriodicNotifierTask>(2, fromWriterToNofirier, dataQueueFromWriterToReader);
            IOtask = std::make_unique<WriteTask>(argv[2], *sharedObject, fromWriterToNofirier, dataQueueFromReaderToWriter);
        }

        MeasureTime([&]() {
            ThreadPool pool(1);
            pool.enqueue(std::move(IOtask));
            pool.enqueue(std::move(Notifier));
        });
    }
}
