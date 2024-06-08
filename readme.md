# EpamTraining course

## File Descriptions
The `Epam-cpp` package exposes next group of files to user:

### Entry point

| File                   |Description|
|:-----------------------|:-|
| [`main.cpp`](main.cpp) |Contains  entry point to `Epam-cpp` application.|

### Library with the logic

| File                                             | Description                                                                                                                                               |
|:-------------------------------------------------|:----------------------------------------------------------------------------------------------------------------------------------------------------------|
| [`task.h`](task.h)                               | The class **ReadTask** reads a buffer from ifstream and pass it to WriteTask. The class **ReadTask** writes a buffer to ofstream and passes it to Reader. |
| [`sharedmemorymanager.h`](sharedmemorymanager.h) | The class **SharedMemoryManager** operates over shared memory object, defines who the current thread is (reader/writer), makes process-safe buffers.      | |