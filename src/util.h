#pragma once

#include <memory>
#include <semaphore.h>

enum class Type : uint8_t { reader = 0, writer = 1, none = 2 };

void EraseFile(const char* fileName);