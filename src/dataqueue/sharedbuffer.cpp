#include "sharedbuffer.h"

SharedBuffer::SharedBuffer(size_t index)
    : m_index(index)
{
}

[[nodiscard]] char* SharedBuffer::Data() noexcept
{
    return m_data;
}

size_t& SharedBuffer::Size() noexcept
{
    return m_length;
}

uint32_t SharedBuffer::Index() const noexcept
{
    return m_index;
}