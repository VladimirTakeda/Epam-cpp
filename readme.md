# EpamTraining course

## File Descriptions
The `Epam-cpp` package exposes next group of files to user:

### Entry point

| File                   |Description|
|:-----------------------|:-|
| [`main.cpp`](main.cpp) |Contains  entry point to `Epam-cpp` application.|

### Library with the logic

| File                           | Description                                                                                                                                             |
|:-------------------------------|:--------------------------------------------------------------------------------------------------------------------------------------------------------|
| [`task.h`](task.h)             | The class **ReadTask** reads a buffer from ifstream and pass it to WriteTask. The class **ReadTask** writes a buffer to ofstream and passes it to Reader. |
| [`threadpool.h`](threadpool.h) | The class **ThreadPool** creates n std::threads and evaluetes the tasks from queue untill it is terminated.                                             |
| [`dataqueue.h`](dataqueue.h)  | The class **DataQueue** is a storage that holds at most two buffers and enables to transfer buffers between Reader and Writer task.                     |