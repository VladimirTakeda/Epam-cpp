#include "util.h"

#include <cstring>
#include <fcntl.h>
#include <filesystem>
#include <iostream>
#include <utility>

void EraseFile(const char* fileName)
{
    try {
        if (std::filesystem::remove(fileName)) {
            std::cout << "File deleted successfully" << std::endl;
        } else {
            std::cout << "File not found or could not be deleted" << std::endl;
        }
    } catch (const std::filesystem::filesystem_error& e) {
        std::cerr << "Filesystem error: " << e.what() << std::endl;
    }
}
