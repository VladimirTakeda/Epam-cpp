#include <fstream>

#include "threadpool.h"
#include "task.h"


int main(int argc, char *argv[]){
    if (argc != 3){
        std::cerr << "wrong arguments count" << std::endl;
        return 1;
    }

    std::ofstream out(argv[2]);
    std::ifstream in(argv[1]);
    boost::fibers::buffered_channel<std::vector<char>> chan(16);
    std::unique_ptr<ReadTask> read = std::make_unique<ReadTask>(in, chan);
    std::unique_ptr<WriteTask> write = std::make_unique<WriteTask>(out, chan);
    ThreadPool pool(2);
    pool.enqueue(std::move(read));
    pool.enqueue(std::move(write));
}