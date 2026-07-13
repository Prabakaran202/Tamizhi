import os
import shutil
from pathlib import Path
from setuptools import setup, find_packages

# =========================================================
# 🌟 Cleanup Logic (பழைய தமிழி பைனரிகளை அழித்தல்)
# =========================================================
def clean_old_installation():
    home_dir = Path.home()
    
    # 1. பழைய .tamizhi ஃபோல்டரை அழித்தல்
    tamizhi_dir = home_dir / ".tamizhi"
    if tamizhi_dir.exists():
        print("🗑️ பழைய தமிழி வெர்ஷன் அழிக்கப்படுகிறது...")
        shutil.rmtree(tamizhi_dir, ignore_errors=True)

    # 2. பழைய extract ஃபோல்டரை அழித்தல்
    extract_dir = home_dir / "tamizhi-extract"
    if extract_dir.exists():
        shutil.rmtree(extract_dir, ignore_errors=True)

    # 3. பழைய .tar.gz ஃபைலை அழித்தல்
    archive_file = home_dir / "tamizhi.tar.gz"
    if archive_file.exists():
        try:
            os.remove(archive_file)
        except OSError:
            pass

# Install ஆகும்போதே கிளீன்-அப் லாஜிக்கை இயக்குகிறோம்
clean_old_installation()

# =========================================================
# 🚀 Standard Setup
# =========================================================
setup(
    name="tamizhi",
    version="2.1.6", # 'v' இல்லாமல் எண்கள் மட்டும் கொடுப்பது PyPI ஸ்டாண்டர்ட்
    packages=find_packages(),
    include_package_data=True,
    entry_points={
        "console_scripts": [
            "tamizhi=tamizhi.cli:main",
        ],
    },
)
