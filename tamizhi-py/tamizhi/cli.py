import subprocess
import sys
import os
import urllib.request
import tarfile
from pathlib import Path

BINARY_PATH = Path.home() / ".tamizhi" / "bin" / "tamizhi"
RELEASE_URL = "https://github.com/BackendDeveloperHub/tamizhi/releases/latest/download/tamizhi-linux.tar.gz"

def ensure_binary():
    if not BINARY_PATH.exists():
        BINARY_PATH.parent.mkdir(parents=True, exist_ok=True)
        print("Downloading Tamizhi compiler...")
        urllib.request.urlretrieve(RELEASE_URL, "/tmp/tamizhi.tar.gz")
        with tarfile.open("/tmp/tamizhi.tar.gz") as tar:
            tar.extractall("/tmp/tamizhi-extract")
        # binary copy
        os.system(f"cp /tmp/tamizhi-extract/*/tamizhi {BINARY_PATH}")
        os.chmod(BINARY_PATH, 0o755)

def main():
    ensure_binary()
    subprocess.run([str(BINARY_PATH)] + sys.argv[1:])
