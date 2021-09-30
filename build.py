from setuptools import Extension
from pathlib import Path

build_file_c = [str(cfile) for cfile in Path("pytct/").glob("**/*.c")]
build_file_cpp = [str(cppfile) for cppfile in Path("pytct/").glob("**/*.cpp")]

build_files = build_file_c + build_file_cpp

extensions = [
    Extension(
        "pytct.libtct",
        sources=build_files
    )
]


def build(setup_kwargs):
    """
    This function is mandatory in order to build the extensions.
    """
    setup_kwargs.update({"ext_modules": extensions})
