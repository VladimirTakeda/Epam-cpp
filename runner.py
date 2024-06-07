import subprocess
import numpy as np
from multiprocessing import shared_memory

SHM_NAME = "shared_memory_object"

SIZE = 2 * 1024 * 1024 + 100

try:
    # Создание shared memory object
    shm_fd = shared_memory.SharedMemory(SHM_NAME, create=True, size=SIZE)
    np_array = np.ndarray((SIZE,), dtype=np.uint8, buffer=shm_fd.buf)
    np_array.fill(0)
except Exception as e:
    shm_fd = shared_memory.SharedMemory(SHM_NAME, create=False, size=SIZE)
    shm_fd.close()
    shm_fd.unlink()
    print(f"Error: {e}")
    exit(1)

command1 = "./cmake-build-release/Epam-cpp /home/vladimir/Desktop/Epam-cpp/_720_2100169427.mp4 /home/vladimir/Desktop/Epam-cpp/out.mp4 " + SHM_NAME
command2 = "./cmake-build-release/Epam-cpp /home/vladimir/Desktop/Epam-cpp/_720_2100169427.mp4 /home/vladimir/Desktop/Epam-cpp/out.mp4 " + SHM_NAME

process1 = subprocess.Popen(command1, shell=True)
process2 = subprocess.Popen(command2, shell=True)

process1.wait()
process2.wait()

shm_fd.close()
shm_fd.unlink()

