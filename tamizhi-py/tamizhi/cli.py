import subprocess
import sys
import os
import urllib.request
import tarfile
import shutil
from pathlib import Path

BINARY_PATH = Path.home() / ".tamizhi" / "bin" / "tamizhi"

RELEASE_URL = (
    "https://github.com/BackendDeveloperHub/Tamizhi/blob/main/tamizhi-v2.0.2-linux.tar.gz"
)

def ensure_binary():

    if BINARY_PATH.exists():
        return

    BINARY_PATH.parent.mkdir(parents=True, exist_ok=True)

    archive_path = "/tmp/tamizhi.tar.gz"
    extract_path = "/tmp/tamizhi-extract"

    print("⬇ Downloading Tamizhi compiler...")

    urllib.request.urlretrieve(RELEASE_URL, archive_path)

    if os.path.exists(extract_path):
        shutil.rmtree(extract_path)

    os.makedirs(extract_path, exist_ok=True)

    with tarfile.open(archive_path) as tar:
        tar.extractall(extract_path)

    for root, dirs, files in os.walk(extract_path):
        if "tamizhi" in files:
            binary_src = os.path.join(root, "tamizhi")
            shutil.copy(binary_src, BINARY_PATH)
            break

    os.chmod(BINARY_PATH, 0o755)

    print("✅ Tamizhi installed successfully!")

def main():

    ensure_binary()

    subprocess.run(
        [str(BINARY_PATH)] + sys.argv[1:]
    )

if __name__ == "__main__":
    main()
