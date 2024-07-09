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
            std::cout << std::this_thread::get_id() << " butesRead " << bytesRead << std::endl;
            toSend->Size() = bytesRead;
            m_dataQueueFromIOToEvent.sendBuffer(std::move(toSend));
            toSend = m_dataQueueFromEventToIO.receiveBuffer();
            std::cout << std::this_thread::get_id() << " loop reading " << std::endl;
        }
        m_dataQueueFromIOToEvent.sendBuffer(nullptr);
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
            std::cout << std::this_thread::get_id() << " writer try to get token " << std::endl;
            m_out.write(toSend->Data(), toSend->Size());
            m_dataQueueFromIOToEvent.sendBuffer(std::move(toSend));
            toSend = m_dataQueueFromEventToIO.receiveBuffer();
            std::cout << std::this_thread::get_id() << " loop writing " << std::endl;
        }
        m_dataQueueFromIOToEvent.sendBuffer(nullptr);
        std::cout << std::this_thread::get_id() << " end writing " << std::endl;
    }

private:
    /// @brief erase destination in case of error
    /// don't create method for one invoke
    void CleanUp(){};

private:
    std::ofstream m_out;

    DataQueue& m_dataQueueFromEventToIO;
    DataQueue& m_dataQueueFromIOToEvent;
};

/// Accumulate all the messages from another process to DataQueue
class EventReaderTask final : public Task {
public:
    explicit EventReaderTask(SharedMemoryManager& sharedMemoryManager, DataQueue& dataQueueFromNotifierToIO,
                             InterProcessDataQueue& dataQueueFromSharedMemoryToEventReader)
        : m_sharedMemory(sharedMemoryManager)
        , m_dataQueueFromEventReaderToIO(dataQueueFromNotifierToIO)
        , m_dataQueueFromSharedMemoryToEventReader(dataQueueFromSharedMemoryToEventReader)
    {
    }
    /// @brief grab the data from message queue and puts it to IOQueue
    void Run() override
    {
        while (true) {
            Message message = m_dataQueueFromSharedMemoryToEventReader.receiveData();

            if (message.type == I_HAVE_A_DATA) {
                m_dataQueueFromEventReaderToIO.sendBuffer(m_sharedMemory.GetBufferByIndex(message.bufferIndex));
            }
            if (message.type == I_HAVE_DONE) {
                m_dataQueueFromEventReaderToIO.sendBuffer(nullptr);
                bool res = m_stopSource.request_stop();
                if (!res) {
                    std::cout << std::this_thread::get_id() << " can't stop the threads " << std::endl;
                }
                break;
            }
            if (message.type == I_AM_ALIVE) {
                continue;
            }
        }
        std::cout << std::this_thread::get_id() << " end read Notifier " << std::endl;
    }

private:
    std::stop_source m_stopSource;
    SharedMemoryManager& m_sharedMemory;
    DataQueue& m_dataQueueFromEventReaderToIO;
    InterProcessDataQueue& m_dataQueueFromSharedMemoryToEventReader;
};

/// Accumulate all the messages from another process to DataQueue
class EventWriterTask final : public Task {
public:
    explicit EventWriterTask(DataQueue& dataQueueFromIOToEventWriter,
                             InterProcessDataQueue& dataQueueFromEventWriterToSharedMemory)
        : m_dataQueueFromIOToEventWriter(dataQueueFromIOToEventWriter)
        , m_dataQueueFromEventWriterToSharedMemory(dataQueueFromEventWriterToSharedMemory)
    {
    }
    /// @brief grab the data from message queue and puts it to IOQueue
    void Run() override
    {
        while (true) {
            auto Buffer = m_dataQueueFromIOToEventWriter.receiveBuffer();
            if (Buffer && Buffer->Data())
                m_dataQueueFromEventWriterToSharedMemory.sendData(Message{I_HAVE_A_DATA, ++messageId, Buffer->Index()});
            else {
                m_dataQueueFromEventWriterToSharedMemory.sendData(Message{I_HAVE_DONE, ++messageId, 0});
                break;
            }
        }
        std::cout << std::this_thread::get_id() << " end write Notifier " << std::endl;
    }

private:
    size_t messageId = 0;
    DataQueue& m_dataQueueFromIOToEventWriter;
    InterProcessDataQueue& m_dataQueueFromEventWriterToSharedMemory;
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

std::unique_ptr<Task> CreateEventReaderTask(SharedMemoryManager& sharedMemoryManager, DataQueue& dataQueueFromNotifierToIO,
                                            InterProcessDataQueue& dataQueueFromSharedMemoryToEventReader)
{
    return std::make_unique<EventReaderTask>(sharedMemoryManager, dataQueueFromNotifierToIO,
                                             dataQueueFromSharedMemoryToEventReader);
}

std::unique_ptr<Task> CreateEventWriterTask(DataQueue& dataQueueFromIOToEventWriter,
                                            InterProcessDataQueue& dataQueueFromEventWriterToSharedMemory)
{
    return std::make_unique<EventWriterTask>(dataQueueFromIOToEventWriter, dataQueueFromEventWriterToSharedMemory);
}