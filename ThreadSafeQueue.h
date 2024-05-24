#pragma once

#include <queue>
#include <mutex>

constexpr int BlockSize = 1024 * 1024;

class ThreadSafeQueue {
public:
    ThreadSafeQueue(){
        m_data.resize(2);
        m_sizes.resize(2);
    }

    bool Write(std::ofstream &out){
        std::unique_lock<std::mutex> lock(writeM);

        writeCv.wait(lock, [this]() { return (writeIndex + 1) % 2 != readIndex || isDone; });
        if (isDone)
            return false;

        writeIndex = (writeIndex + 1) % 2;
        bool res = out.write(m_data[writeIndex].data(), m_sizes[writeIndex]).operator bool();
        readCv.notify_one();
        return res;
    }

    bool Read(std::ifstream &in){
        std::unique_lock<std::mutex> lock(readM);

        readCv.wait(lock, [this]() { return readIndex != writeIndex || isDone; });
        if (isDone)
            return false;

        bool res(in.read(m_data[readIndex].data(), BlockSize) || in.gcount() > 0);
        m_sizes[readIndex] = in.gcount();
        readIndex = (readIndex + 1) % 2;
        writeCv.notify_one();
        return res;
    }

    std::vector<std::array<char, BlockSize>> m_data;
    std::vector<size_t> m_sizes;
    alignas(64) std::atomic_int writeIndex = -1;
    alignas(64) std::atomic_int readIndex = 0;
    alignas(64) std::atomic_bool isDone = false;
    std::mutex writeM;
    std::mutex readM;
    std::condition_variable writeCv;
    std::condition_variable readCv;
};
