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


~~пинг - поднятие семафора~~
~~дорисовать определение кто есть кто в отдельной схеме~~
~~RAII над семафорами~~
~~очередь в отдельный класс~~
понять, как общаться с помощью двух семафоров ???
~~понять как из одного потока (healthChecker) остановить все остальные потоки~~
~~понять, как и куда запихивать первые два индекса в ридере~~
~~понять, нужен ли нам вообще NotifierTask~~
если мы делаем acknowledgement, то в текущей реализации в нём нет смысла. Так как доставка сообщения подтверждается семафором, то есть если сообщение по какой-то причине не дошло, то мы не сможем записать новое
Останавливать потоки и из других классов тоже, когда мы завершаем чтение/запись, мы должны остановить пинги
stop_token + cv?
~~Мы посылаем, когда хотим посылать, а не когда бафер свободен. Читаем, когда есть что читать, а не когда нам нужно читать.~~
Делаешь stop_source, передаёшь его в health-check поток, а токены раздаёшь управляемым потокам.
~~We need a new task than will grab a new value from buffer queue whenever it appears.~~
~~That task takes a value from shared memory and puts it to NotifierToIOQueue~~
Кто должен удалять семафоры? Если удаляет тот, кто первый завершился, то в другом процессе скорее всего получил исключение из-за доступа к удалённому семафору