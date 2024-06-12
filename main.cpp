#include <fstream>
#include <iostream>
#include <memory>
#include <filesystem>
#include <csignal>
#include <thread>

#include "task.h"
#include "dataqueue.h"
#include "sharedmemorymanager.h"

void removeFile(char* fileName) {
    try {
        if (std::filesystem::remove(fileName)) {
            std::cout << "File deleted successfully" << std::endl;
        } else {
            std::cout << "File not found or could not be deleted" << std::endl;
        }
    } catch (const std::filesystem::filesystem_error& e) {
        std::cerr << "Filesystem error: " << e.what() << std::endl;
    }
}

int main(int argc, char *argv[]){
    if (argc != 4){
        std::cerr << "wrong arguments count" << std::endl;
        return 1;
    }

    {
        std::unique_ptr<SharedMemoryManager> sharedObject;

        try {
            sharedObject = std::make_unique<SharedMemoryManager>(argv[3]);
        }
        catch(...){
            std::cout << "shared memory wrapper constructor fails" << std::endl;
        }

        std::unique_ptr<std::ifstream> in;
        std::unique_ptr<std::ofstream> out;

        std::unique_ptr<Task> task;

        DataQueue dataQueueFromReaderToWriter(sharedObject->GetFromReaderToWriterPipe(), sharedObject->GetSharedMemoryPtr());
        DataQueue dataQueueFromWriterToReader(sharedObject->GetFromWriterToReaderPipe(), sharedObject->GetSharedMemoryPtr());

        if (sharedObject->WhoAmI() == Type::reader){
            in = std::make_unique<std::ifstream>(argv[1]);
            std::cout << std::this_thread::get_id() << " I am reader" << std::endl;

            task = std::make_unique<ReadTask>(*in, dataQueueFromReaderToWriter, dataQueueFromWriterToReader);
        }
        else{
            std::cout << std::this_thread::get_id() << " I am writer" << std::endl;
            removeFile(argv[2]);


            dataQueueFromWriterToReader.sendData(sharedObject->GetNext());
            dataQueueFromWriterToReader.sendData(sharedObject->GetNext());

            out = std::make_unique<std::ofstream>(argv[2]);
            task = std::make_unique<WriteTask>(*out, dataQueueFromReaderToWriter, dataQueueFromWriterToReader);
        }

        auto start = std::chrono::high_resolution_clock::now();
        {
            task->Run();
        }
        auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> duration = end - start;
        std::cout << "Queue: " << std::chrono::duration_cast<std::chrono::microseconds>(duration).count() << " microsec" << std::endl;
    }
}