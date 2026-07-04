import subprocess
import sys
import os
import urllib.request
import tarfile
import shutil
from pathlib import Path

BINARY_PATH = Path.home() / ".tamizhi" / "bin" / "tamizhi"
RELEASE_URL = (
    "https://github.com/BackendDeveloperHub/Tamizhi/raw/main/tamizhi-v2.0.2-linux.tar.gz"
)

def ensure_binary():
    if BINARY_PATH.exists():
        return

    BINARY_PATH.parent.mkdir(parents=True, exist_ok=True)

    #archive_path = "/tmp/tamizhi.tar.gz"
    #extract_path = "/tmp/tamizhi-extract"
        # பழைய வரிகளை நீக்கிவிட்டு, இதைச் சேர்க்கவும்:
    archive_path = Path.home() / "tamizhi.tar.gz"
    extract_path = Path.home() / "tamizhi-extract"
    

    print("⬇ Downloading Tamizhi compiler...")

    req = urllib.request.Request(
        RELEASE_URL, 
        headers={'User-Agent': 'Mozilla/5.0'}
    )
    
    with urllib.request.urlopen(req) as response, open(archive_path, 'wb') as out_file:
        shutil.copyfileobj(response, out_file)

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

# இந்த main() பங்க்ஷன் கட்டாயம் இருக்க வேண்டும்!
def main():
    ensure_binary()
    subprocess.run([str(BINARY_PATH)] + sys.argv[1:])

if __name__ == "__main__":
    main()
