#include <iostream>

#include "task.h"

constexpr int BlockSize = 1024 * 1024;
constexpr size_t Empty = 0;

Task::~Task() = default;


ReadTask::ReadTask(std::ifstream &in, DataQueue &dataQueueFromReaderToWriter, DataQueue &dataQueueFromWriterToReader) : m_in(in),
                                                                  m_dataQueueFromReaderToWriter(dataQueueFromReaderToWriter), m_dataQueueFromWriterToReader(dataQueueFromWriterToReader) {

}

void ReadTask::Run() {
    auto toSend = m_dataQueueFromWriterToReader.receiveData();
    while (m_in.read(toSend->data, BlockSize) || m_in.gcount() > 0) {
        std::size_t bytesRead = m_in.gcount();
        toSend->length = bytesRead;

        m_dataQueueFromReaderToWriter.sendData(std::move(toSend));
        toSend = m_dataQueueFromWriterToReader.receiveData();
    }
    toSend->length = Empty;
    m_dataQueueFromReaderToWriter.sendData(std::move(toSend));
    std::cout << "endRead\n";
}

WriteTask::WriteTask(std::ofstream &out, DataQueue &dataQueueFromReaderToWriter, DataQueue &dataQueueFromWriterToReader) : m_out(out),
                                                                    m_dataQueueFromReaderToWriter(dataQueueFromReaderToWriter), m_dataQueueFromWriterToReader(dataQueueFromWriterToReader) {

}

void WriteTask::Run() {
    auto toSend = m_dataQueueFromReaderToWriter.receiveData();
    while (toSend->length) {
        m_out.write(toSend->data, toSend->length);
        m_dataQueueFromWriterToReader.sendData(std::move(toSend));
        toSend = m_dataQueueFromReaderToWriter.receiveData();
    }
    std::cout << "endWrite\n";
}