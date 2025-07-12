# -*- coding: utf-8 -*-
from setuptools import setup, Extension
from pathlib import Path

build_file_c = [str(cfile) for cfile in Path("libtct/").glob("**/*.c")] + ["pitct/libtct.c"]
build_file_cpp = [str(cppfile) for cppfile in Path("libtct/").glob("**/*.cpp")]
build_files = build_file_c + build_file_cpp

extensions = [
    Extension(
        "pitct.libtct",
        sources=build_files,
        # extra_compile_args=['-O0']
    )
]

# lldb debug #
# put libtct.***.so in pitct directory
# lldb venv/bin/python
# br set --file program.c --line 100
# run sample.py
setup(ext_modules=extensions)