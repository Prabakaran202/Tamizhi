from setuptools import setup, find_packages

setup(
    name="tamizhi",
    version="v0.1.0",
    packages=find_packages(),
    include_package_data=True,
    entry_points={
        "console_scripts": [
            "tamizhi=tamizhi.cli:main",
        ],
    },
)
