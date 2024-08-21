import os
import subprocess
import requests
import pytest
import signal
import time

url = "https://hd.trn.su/720/2100169427.mp4?md5=h3rzwagskVzQZV2qT1neUQ&time=1717809174&d=1"
local_filename = "downloaded_video.mp4"


def download_file(url, local_filename):
    if not os.path.exists(local_filename):
        with requests.get(url, stream=True) as response:
            response.raise_for_status()
            with open(local_filename, 'wb') as file:
                for chunk in response.iter_content(chunk_size=8192):
                    file.write(chunk)
        print(f"File has been downloaded and saved as {local_filename}")
    else:
        print(f"File {local_filename} already exists, skip downloading.")


def is_named_objects_empty(sharedMemoryObjectName):
    base_path = "/dev/shm/"

    for i in range(1, 7):
        semaphore_name = f"{sharedMemoryObjectName}{i}.sem"
        semaphore_path = os.path.join(base_path, semaphore_name)
        if os.path.exists(semaphore_path):
            return False

    shm_name = f"{sharedMemoryObjectName}.obj"
    shm_path = os.path.join(base_path, shm_name)
    if os.path.exists(shm_path):
        return False
    return True


@pytest.fixture
def clean_up_download():
    """Фикстура для очистки скачанного файла после теста"""
    yield
    local_filename = "downloaded_video.mp4"
    if os.path.exists(local_filename):
        os.remove(local_filename)


@pytest.fixture
def binary_path(request):
    return request.config.getoption("--binary-path")


@pytest.fixture
def project_dir(request):
    return request.config.getoption("--project-dir")


def test_download_file(clean_up_download):
    download_file(url, local_filename)

    assert os.path.exists(local_filename), "File was not downloaded successfully."


def test_binary_runs(binary_path, project_dir):
    download_file(url, f"../{local_filename}")
    shm_name = "shared_memory_object_default"

    command1 = f"{binary_path} {project_dir}/{local_filename} {project_dir}/out1.mp4 " + shm_name
    command2 = f"{binary_path} {project_dir}/{local_filename} {project_dir}/out1.mp4 " + shm_name

    process1 = subprocess.Popen(command1, shell=True)
    process2 = subprocess.Popen(command2, shell=True)

    process1.wait()
    process2.wait()

    assert is_named_objects_empty(shm_name), "Named object has not been deleted"


def test_duplicate_runs(binary_path, project_dir):
    download_file(url, f"../{local_filename}")
    shm_name = "shared_memory_object_duplicate"

    command1 = f"{binary_path} {project_dir}/{local_filename} {project_dir}/out1.mp4 " + shm_name
    command2 = f"{binary_path} {project_dir}/{local_filename} {project_dir}/out1.mp4 " + shm_name
    command3 = f"{binary_path} {project_dir}/{local_filename} {project_dir}/out1.mp4 " + shm_name
    command4 = f"{binary_path} {project_dir}/{local_filename} {project_dir}/out1.mp4 " + shm_name

    process1 = subprocess.Popen(command1, shell=True)
    process2 = subprocess.Popen(command2, shell=True)
    process3 = subprocess.Popen(command3, shell=True)
    process4 = subprocess.Popen(command4, shell=True)

    process1.wait()
    process2.wait()
    process3.wait()
    process4.wait()

    assert is_named_objects_empty(shm_name), "Named object has not been deleted"


def test_parallel_runs(binary_path, project_dir):
    download_file(url, f"../{local_filename}")
    shm_name_4 = "shared_memory_object_duplicate_4"
    shm_name_5 = "shared_memory_object_duplicate_5"

    command1 = f"{binary_path} {project_dir}/{local_filename} {project_dir}/out1.mp4 " + shm_name_4
    command2 = f"{binary_path} {project_dir}/{local_filename} {project_dir}/out1.mp4 " + shm_name_4
    command3 = f"{binary_path} {project_dir}/{local_filename} {project_dir}/out2.mp4 " + shm_name_5
    command4 = f"{binary_path} {project_dir}/{local_filename} {project_dir}/out2.mp4 " + shm_name_5

    process1 = subprocess.Popen(command1, shell=True)
    process2 = subprocess.Popen(command2, shell=True)
    process3 = subprocess.Popen(command3, shell=True)
    process4 = subprocess.Popen(command4, shell=True)

    process1.wait()
    process2.wait()
    process3.wait()
    process4.wait()

    assert is_named_objects_empty(shm_name_4), "Named object has not been deleted"
    assert is_named_objects_empty(shm_name_5), "Named object has not been deleted"
