#include <fstream>
#include <iostream>
#include <memory>

#include "threadpool.h"
#include "task.h"
#include "DataQueue.h"

int main(int argc, char *argv[]){
    if (argc != 3){
        std::cerr << "wrong arguments count" << std::endl;
        return 1;
    }

    try {
        if (std::filesystem::remove(argv[2])) {
            std::cout << "File deleted successfully" << std::endl;
        } else {
            std::cout << "File not found or could not be deleted" << std::endl;
        }
    } catch (const std::filesystem::filesystem_error& e) {
        std::cerr << "Filesystem error: " << e.what() << std::endl;
    }

    {
        std::ofstream out(argv[2]);
        std::ifstream in(argv[1]);

        boost::fibers::buffered_channel<std::vector<char>> chan(16);
        std::unique_ptr<Task> read = std::make_unique<ReadTaskBoost>(in, chan);
        std::unique_ptr<Task> write = std::make_unique<WriteTaskBoost>(out, chan);
        auto start = std::chrono::high_resolution_clock::now();
        {
            ThreadPool pool(1);
            pool.enqueue(std::move(read));
            pool.enqueue(std::move(write));
        }
        auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> duration = end - start;
        std::cout << "Boost: " << std::chrono::duration_cast<std::chrono::microseconds>(duration).count() << " microces" << std::endl;
    }

    try {
        if (std::filesystem::remove(argv[2])) {
            std::cout << "File deleted successfully" << std::endl;
        } else {
            std::cout << "File not found or could not be deleted" << std::endl;
        }
    } catch (const std::filesystem::filesystem_error& e) {
        std::cerr << "Filesystem error: " << e.what() << std::endl;
    }

    {
        std::ofstream out(argv[2]);
        std::ifstream in(argv[1]);


        DataQueue dataQueueFromReaderToWriter;
        DataQueue dataQueueFromWriterToReader;
        dataQueueFromWriterToReader.sendData(std::make_unique<Buffer>());
        dataQueueFromWriterToReader.sendData(std::make_unique<Buffer>());

        std::unique_ptr<Task> read = std::make_unique<ReadTask>(in, dataQueueFromReaderToWriter, dataQueueFromWriterToReader);
        std::unique_ptr<Task> write = std::make_unique<WriteTask>(out, dataQueueFromReaderToWriter, dataQueueFromWriterToReader);
        auto start = std::chrono::high_resolution_clock::now();
        {
            ThreadPool pool(1);
            pool.enqueue(std::move(read));
            pool.enqueue(std::move(write));
        }
        auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> duration = end - start;
        std::cout << "Queue: " << std::chrono::duration_cast<std::chrono::microseconds>(duration).count() << " microsec" << std::endl;
    }
}