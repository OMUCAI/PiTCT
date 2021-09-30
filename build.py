from setuptools import Extension
from pathlib import Path
import tarfile
from pathlib import Path
import os
import glob
import shutil

build_file_c = [str(cfile) for cfile in Path("pytct/").glob("**/*.c")]
build_file_cpp = [str(cppfile) for cppfile in Path("pytct/").glob("**/*.cpp")]

build_files = build_file_c + build_file_cpp

extensions = [
    Extension(
        "pytct.libtct",
        sources=build_files
    )
]

def update_sdist():
    # search latest sdist file
    files = glob.glob("dist/*.tar.gz")
    files.sort(key=os.path.getmtime, reverse=True)
    if len(files) < 1:
        return
    filename = files[0]

    # extract content
    with tarfile.open(filename, mode='r:gz') as tar:
        tar.extractall("dist/sdist")
    
    # copy source
    dir = glob.glob("dist/sdist/PyTCT-*")[0]
    shutil.copytree("pytct/libtct", f"{dir}/pytct/libtct")

    # compress sdist
    with tarfile.open(filename, "w:gz") as tar:
        tar.add("dist/sdist", "/")
    
    # remove tmp content file
    shutil.rmtree("dist/sdist")

def build(setup_kwargs):
    """
    This function is mandatory in order to build the extensions.
    """
    print("Add C Source to sdist")
    update_sdist()
    setup_kwargs.update({"ext_modules": extensions})
