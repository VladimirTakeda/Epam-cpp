import os
import subprocess
import requests
import pytest
import signal
import time

url = "https://hd.trn.su/720/2100169427.mp4?md5=h3rzwagskVzQZV2qT1neUQ&time=1717809174&d=1"


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
    url = "https://hd.trn.su/720/2100169427.mp4?md5=h3rzwagskVzQZV2qT1neUQ&time=1717809174&d=1"
    local_filename = "downloaded_video.mp4"
    download_file(url, local_filename)

    assert os.path.exists(local_filename), "File was not downloaded successfully."


def test_binary_runs(binary_path, project_dir):
    local_filename = "downloaded_video_1.mp4"
    download_file(url, f"../{local_filename}")
    shm_name = "shared_memory_object_default"

    command1 = f"{binary_path} {project_dir}/{local_filename} {project_dir}/out1.mp4 " + shm_name
    command2 = f"{binary_path} {project_dir}/{local_filename} {project_dir}/out1.mp4 " + shm_name

    process1 = subprocess.Popen(command1, shell=True)
    process2 = subprocess.Popen(command2, shell=True)

    process1.wait()
    process2.wait()

    assert process1.returncode == 0, "Process 1 did not terminate successfully."
    assert process2.returncode == 0, "Process 2 did not terminate successfully."


def test_process_suspend_resume(binary_path, project_dir):
    local_filename = "downloaded_video_2.mp4"
    download_file(url, f"../{local_filename}")
    shm_name = "shared_memory_object_suspend"

    command1 = f"{binary_path} {project_dir}/{local_filename} {project_dir}/out2.mp4 " + shm_name
    command2 = f"{binary_path} {project_dir}/{local_filename} {project_dir}/out2.mp4 " + shm_name

    process1 = subprocess.Popen(command1, shell=True)
    process2 = subprocess.Popen(command2, shell=True)

    os.kill(process2.pid, signal.SIGSTOP)
    time.sleep(2)
    os.kill(process2.pid, signal.SIGCONT)

    process1.wait()
    process2.wait()

    assert process1.returncode == 0, "Process 1 did not complete successfully after being stopped and continued."
    assert process2.returncode == 0, "Process 2 did not complete successfully."
