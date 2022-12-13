import subprocess
import os


if __name__ == "__main__":
    with open(os.devnull, "w") as f:
        subprocess.Popen(["/usr/bin/shellcode"], stdout=f, stderr=f)