#include <filesystem>
#include <iostream>

#include "util.h"

bool IsPing(size_t value){
    return std::numeric_limits<size_t>::max() == value;
}

void EraseFile(const char *fileName) {
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
