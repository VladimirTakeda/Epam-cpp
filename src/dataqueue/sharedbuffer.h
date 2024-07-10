#pragma once

#include <memory>

/// A buffer (on shared memory) for processes communication
/// Placement new
/// Try to connect it with the index
struct SharedBuffer {
    // because default constructor initilize m_length with zero
    explicit SharedBuffer(size_t index);
    [[nodiscard]] char* Data() noexcept;
    size_t& Size() noexcept;
    [[nodiscard]] uint32_t Index() const noexcept;

protected:
    char m_data[1024 * 1024]; // can't initilize, because writer need the data from shared memory
    size_t m_length; // can't initilize, bacause we need the length from shared_memory
    uint32_t m_index = 0;
};

class CustomDeleter {
public:
    void operator()(SharedBuffer* ptr) const { ptr->~SharedBuffer(); }
};

typedef std::unique_ptr<SharedBuffer, CustomDeleter> toSend;

enum MessageType : uint8_t {
    I_AM_ALIVE    = 1, // send if we can't take the data from dataqueue
    I_SEE_IT      = 2,
    I_HAVE_A_DATA = 3, // comes with unique message id and buffer index
    I_SAW_A_DATA  = 4, // comes with the same message id as in I_HAVE_A_DATA
    I_HAVE_DONE   = 5,
};

struct Message {
    MessageType type;
    uint64_t messageId;
    uint32_t bufferIndex;
};