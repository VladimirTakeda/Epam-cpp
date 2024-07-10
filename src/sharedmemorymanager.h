#pragma once

#include "dataqueue/sharedbuffer.h"
#include "util.h"

#include <string>

class SemWrapper;

constexpr int SHM_SIZE = 2 * 1024 * 1024 + 100;

// what is mac os stack size - 8176 byte
// Natural alignment, for example, on a word boundary at 0x1004. The ARM compiler normally aligns variables and pads structures so
// that these items are accessed efficiently using LDR and STR instructions. Natural alignment is when an object is aligned to its
// size. For example, a 32-bit integer is naturally aligned when it is 4-byte aligned. For most types on most architectures,
// natural alignment is the only requirement. For example, on Linux/x86-64, the ABI requires only natural alignment: int is 4-byte
// aligned, long and pointers are 8-byte aligned.

/// A class to manipulate shared memory object
class SharedMemoryManager {
public:
    SharedMemoryManager() = default;
    /// @brief opens semaphores, link shared memory in this process, set the process type (reader/writer)
    explicit SharedMemoryManager(const std::string& semaphorePreffixName, const char* SharedObjName);
    /// @brief return buffer from shared memory
    toSend GetBufferByIndex(size_t index);

    [[nodiscard]] Message* GetQueueByIndex(size_t index) const;

    bool InitializeSharedMemory(const std::string& semaphoreName, bool isCreator);

    bool OpenSharedMemory(const std::string& semaphorePreffixName, const char* SharedObjName);
    /// @brief decrement the number of active processes, unlink shared memory object and close semaphore if nesessary
    ~SharedMemoryManager();
    /// @brief return the type of current process (reader/writer/unknown)
    [[nodiscard]] Type WhoAmI() const;

private:
    Type type = Type::none;

    std::unique_ptr<SemWrapper> m_writerSem;
    std::unique_ptr<SemWrapper> m_memorySem;
    char* m_shmPtr{};
    int m_shmFd{};
};
