#pragma once

#include <cstring>
#include <errno.h>
#include <fcntl.h>
#include <functional>
#include <iostream>
#include <sys/mman.h>
#include <vector>

/// Singletone class to delete all created linux objects in std::terminate
class GarbageCollector {
    std::vector<std::pair<std::string, int (*)(const char*)>> toDelete;

    GarbageCollector()  = default;
    ~GarbageCollector() = default;

    GarbageCollector(const GarbageCollector&)            = delete;
    GarbageCollector& operator=(const GarbageCollector&) = delete;

public:
    static GarbageCollector& GetInstance()
    {
        static GarbageCollector instance;
        return instance;
    }

    void Push(const char* name, int (*deleter)(const char*))
    {
        toDelete.emplace_back(name, deleter);
        std::cout << "curr delete size(): " << toDelete.size() << std::endl;
    }

    void CleanObjects()
    {
        for (auto& [objName, deleter] : toDelete) {
            int res = deleter(objName.c_str());
            if (res == -1)
                fprintf(stderr, "Error unlinking shared memory: %s\n", strerror(errno));
            std::cout << "objName " << objName << " " << res << std::endl;
        }
        toDelete.clear();
    }
};