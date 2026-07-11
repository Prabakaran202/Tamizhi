import subprocess
import sys
import os
import urllib.request
import tarfile
import shutil
from pathlib import Path

BINARY_PATH = Path.home() / ".tamizhi" / "bin" / "tamizhi"
# குறிப்பு: நீங்கள் புதிய வெர்ஷன் ரிலீஸ் செய்யும்போது இந்த URL-ஐ மாற்றிக்கொள்ளலாம் (உதா: v2.0.9)
RELEASE_URL = (
    "https://github.com/BackendDeveloperHub/Tamizhi/raw/main/tamizhi-v2.1.6-linux.tar.gz"
)

def ensure_binary():
    if BINARY_PATH.exists():
        return

    BINARY_PATH.parent.mkdir(parents=True, exist_ok=True)

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

    # --- 🌟 புதிய குளோபல் பாத் லாஜிக் (Binary & Core Folder) 🌟 ---
    CORE_PATH = Path.home() / ".tamizhi" / "core"

    for root, dirs, files in os.walk(extract_path):
        # 1. தமிழி பைனரியை தேடி நகர்த்துதல்
        if "tamizhi" in files:
            binary_src = os.path.join(root, "tamizhi")
            shutil.copy(binary_src, BINARY_PATH)
        
        # 2. கம்பைலருக்குத் தேவையான 'core' ஃபோல்டரை தேடி நகர்த்துதல் (பழைய கோடில் இது விடுபட்டிருந்தது)
        if "core" in dirs:
            core_src = os.path.join(root, "core")
            if CORE_PATH.exists():
                shutil.rmtree(CORE_PATH)
            shutil.copytree(core_src, CORE_PATH)

    os.chmod(BINARY_PATH, 0o755)
    print("✅ Tamizhi installed successfully!")

# இந்த main() பங்க்ஷன் கட்டாயம் இருக்க வேண்டும்!
def main():
    # 🌟 யூசர் 'tamizhi upgrade' என்று கொடுத்தால்...
    if len(sys.argv) > 1 and sys.argv[1] == "upgrade":
        print("🔄 தமிழி புதிய வெர்ஷனுக்கு அப்டேட் செய்யப்படுகிறது...")
        tamizhi_folder = Path.home() / ".tamizhi"
        
        # பழைய ஃபோல்டரை முழுமையாக அழிக்கிறோம்
        if tamizhi_folder.exists():
            shutil.rmtree(tamizhi_folder, ignore_errors=True)
            
        # புதிதாக டவுன்லோட் செய்யச் சொல்கிறோம்
        ensure_binary()
        print("🎉 தமிழி வெற்றிகரமாக அப்டேட் செய்யப்பட்டது! (Updated to latest version)")
        return

    # வழக்கம் போல் நடக்கும் வேலைகள்...
    ensure_binary()
    subprocess.run([str(BINARY_PATH)] + sys.argv[1:])

if __name__ == "__main__":
    main()
