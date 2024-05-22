# EpamTraining course

## File Descriptions
The `Epam-cpp` package exposes next group of files to user:

### Entry point

| File                   |Description|
|:-----------------------|:-|
| [`main.cpp`](main.cpp) |Contains  entry point to `Epam-cpp` application.|

### Library with the logic

| File                           | Description                                                                                                                                                                                    |
|:-------------------------------|:-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------|
| [`task.h`](task.h)             | The class **ReadTask** reads a block of bytes from ifstream and stores it in buffered_channel. The class **WriteTask** reads a block of bytes from buffered_channel and writes it to ofstream. |
| [`threadpool.h`](threadpool.h) | The class **ThreadPool** creates n std::threads and evaluetes the tasks from queue untill it is terminated.                                                                                    |