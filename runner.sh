#!/bin/bash

SHM_NAME="shared_memory_object"
SIZE=$((2 * 1024 * 1024 + 2))

if ! shm_fd=$(shm_open "$SHM_NAME" O_CREAT | O_RDWR 2>/dev/null); then
    echo "Error creating shared memory object"
    exit 1
fi

if ! ftruncate "$shm_fd" "$SIZE"; then
    echo "Error setting size of shared memory object"
    exit 1
fi

./cmake-build-release/Epam-cpp "/Users/vovatakeda/Desktop/CPlusPlusProjects/Epam-cpp/_720_2121269575.mp4" "/Users/vovatakeda/Desktop/CPlusPlusProjects/Epam-cpp/out.mp4" "$SHM_NAME" &
./cmake-build-release/Epam-cpp "/Users/vovatakeda/Desktop/CPlusPlusProjects/Epam-cpp/_720_2121269575.mp4" "/Users/vovatakeda/Desktop/CPlusPlusProjects/Epam-cpp/out.mp4" "$SHM_NAME" &

wait

shm_unlink "$SHM_NAME"