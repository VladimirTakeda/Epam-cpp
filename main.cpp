#include "src/dataqueue/sharedbuffer.h"
#include "src/sharedmemorymanager.h"
#include "src/task.h"
#include "src/threadpool.h"
#include "src/utils/garbadgecollector.h"
#include "src/utils/util.h"

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

void MeasureTime(const std::function<void()>& func)
{
    auto start = std::chrono::high_resolution_clock::now();
    func();
    auto end                               = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> duration = end - start;
    std::cout << "Queue: " << std::chrono::duration_cast<std::chrono::microseconds>(duration).count() << " microsec" << std::endl;
}

bool ValidateArguments(int argc, char* argv[])
{
    if (argc != 4) {
        std::cerr << "wrong arguments count" << std::endl;
        return false;
    }

    if (!std::filesystem::is_regular_file(argv[1])) {
        std::cerr << "wrong source file path" << std::endl;
        return false;
    }

    return true;
}

void handleTerminate()
{
    std::cout << "Terminate Here" << std::endl;
    GarbageCollector::GetInstance().CleanObjects();
    std::abort();
}

void handleSigint(int signum)
{
    std::cout << "Sigint Here" << std::endl;
    GarbageCollector::GetInstance().CleanObjects();
    std::abort();
}

int main(int argc, char* argv[])
{
    std::set_terminate(handleTerminate);
    std::signal(SIGINT, handleSigint);
    if (!ValidateArguments(argc, argv)) {
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

        SharedQueueBuffer firstBuf(2, sharedObject->GetQueueByIndex(0), semPrefix + '3', semPrefix + '4',
                                   sharedObject->WhoAmI() == Type::reader);
        SharedQueueBuffer secondBuf(2, sharedObject->GetQueueByIndex(1), semPrefix + '5', semPrefix + '6',
                                    sharedObject->WhoAmI() == Type::reader);

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
            GarbageCollector::GetInstance().Push(argv[2], std::remove);

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
