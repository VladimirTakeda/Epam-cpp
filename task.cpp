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


ReadTaskBoost::ReadTaskBoost(std::ifstream &in, boost::fibers::buffered_channel<std::vector<char>> &chan) : m_in(in),
                                                                                                  m_chan(chan) {

}

void ReadTaskBoost::Run() {
    std::vector<char> buf(BlockSize);

    while (m_in.read(buf.data(), BlockSize) || m_in.gcount() > 0) {
        std::size_t bytesRead = m_in.gcount();
        buf.resize(bytesRead);
        m_chan.push(buf);
    }
    std::cout << "endReadBoost\n";
    m_chan.close();
}

WriteTaskBoost::WriteTaskBoost(std::ofstream &out, boost::fibers::buffered_channel<std::vector<char>> &chan) : m_out(out),
                                                                                                     m_chan(chan) {

}

void WriteTaskBoost::Run() {
    std::vector<char> buf;
    while (boost::fibers::channel_op_status::success == m_chan.pop(buf)) {
        m_out.write(buf.data(), buf.size());
    }
    std::cout << "endWriteBoost\n";
}