#include "task.h"

#include <iostream>
#include <stdexcept>
#include <thread>

constexpr int BlockSize = 1024 * 1024;
constexpr size_t Empty  = 0;

size_t Task::ID()
{
    return m_ID;
}

constexpr nullptr_t StopSignal = nullptr;

/// ifstream Reader
///
/// reader can read 2 buffers from the beginning independently from writer
class ReadTask final : public Task {
public:
    explicit ReadTask(const char* const fileName, DataQueue& dataQueueFromEventToIO, DataQueue& dataQueueFromIOToEvent)
        : m_in(fileName)
        , m_dataQueueFromEventToIO(dataQueueFromEventToIO)
        , m_dataQueueFromIOToEvent(dataQueueFromIOToEvent)
    {
    }
    /// @brief takes free buffer index from other process, if it's valid do read task and push the index to Notifier via
    /// TimedDataQueue
    void Run() override
    {
        std::cout << std::this_thread::get_id() << " start reading " << std::endl;
        auto toSend = m_dataQueueFromEventToIO.receiveBuffer();
        while (toSend && (m_in.read(toSend->Data(), BlockSize) || m_in.gcount() > 0)) {
            std::size_t bytesRead = m_in.gcount();
            // std::cout << std::this_thread::get_id() << " butesRead " << bytesRead << std::endl;
            toSend->Size() = bytesRead;
            m_dataQueueFromIOToEvent.sendBuffer(std::move(toSend));
            toSend = m_dataQueueFromEventToIO.receiveBuffer();
            // std::cout << std::this_thread::get_id() << " loop reading " << std::endl;
        }
        m_dataQueueFromIOToEvent.sendBuffer(StopSignal);
        std::cout << std::this_thread::get_id() << " end reading " << std::endl;
    }

private:
    std::ifstream m_in;

    DataQueue& m_dataQueueFromEventToIO;
    DataQueue& m_dataQueueFromIOToEvent;
};

/// ofstream Writer
class WriteTask final : public Task {
public:
    WriteTask(const char* const fileName, DataQueue& dataQueueFromEventToIO, DataQueue& dataQueueFromIOToEvent)
        : m_out(fileName)
        , m_dataQueueFromEventToIO(dataQueueFromEventToIO)
        , m_dataQueueFromIOToEvent(dataQueueFromIOToEvent)
    {
    }
    /// @brief takes free buffer index from other process, if it's valid do write task and push the index to Notifier via
    /// TimedDataQueue
    void Run() override
    {
        std::cout << std::this_thread::get_id() << " start writing " << std::endl;
        auto toSend = m_dataQueueFromEventToIO.receiveBuffer();
        while (toSend && toSend->Size()) {
            m_out.write(toSend->Data(), toSend->Size());
            m_dataQueueFromIOToEvent.sendBuffer(std::move(toSend));
            toSend = m_dataQueueFromEventToIO.receiveBuffer();
            // std::cout << std::this_thread::get_id() << " loop writing " << std::endl;
        }
        m_dataQueueFromIOToEvent.sendBuffer(StopSignal);
        std::cout << std::this_thread::get_id() << " end writing " << std::endl;
    }

private:
    /// @brief erase destination in case of error
    /// don't create method for one invoke
    void CleanUp() {};

private:
    std::ofstream m_out;

    DataQueue& m_dataQueueFromEventToIO;
    DataQueue& m_dataQueueFromIOToEvent;
};

std::unique_ptr<Task> CreateWriteTask(const char* const fileName, DataQueue& dataQueueFromEventToIO,
                                      DataQueue& dataQueueFromIOToEvent)
{
    return std::make_unique<WriteTask>(fileName, dataQueueFromEventToIO, dataQueueFromIOToEvent);
}

std::unique_ptr<Task> CreateReadTask(const char* const fileName, DataQueue& dataQueueFromEventToIO,
                                     DataQueue& dataQueueFromIOToEvent)
{
    return std::make_unique<ReadTask>(fileName, dataQueueFromEventToIO, dataQueueFromIOToEvent);
}