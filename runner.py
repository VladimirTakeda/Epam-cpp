import os
import subprocess
import requests
import argparse
import numpy as np
from multiprocessing import shared_memory

SHM_NAME = "shared_memory_object"

parser = argparse.ArgumentParser()

parser.add_argument('binary', type=str)
parser.add_argument('projectDir', type=str)

args = parser.parse_args()

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

url = "https://hd.trn.su/720/2100169427.mp4?md5=h3rzwagskVzQZV2qT1neUQ&time=1717809174&d=1"
local_filename = "downloaded_video.mp4"

download_file(url, local_filename)

command1 = f"{args.binary} {args.projectDir}/{local_filename} {args.projectDir}/out.mp4 " + SHM_NAME
command2 = f"{args.binary} {args.projectDir}/{local_filename} {args.projectDir}/out.mp4 " + SHM_NAME

process1 = subprocess.Popen(command1, shell=True)
process2 = subprocess.Popen(command2, shell=True)

process1.wait()
process2.wait()

