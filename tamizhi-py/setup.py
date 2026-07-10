from setuptools import setup, find_packages

setup(
    name="tamizhi",
    version="v2.1.1",
    packages=find_packages(),
    include_package_data=True,
    entry_points={
        "console_scripts": [
            "tamizhi=tamizhi.cli:main",
        ],
    },
)
