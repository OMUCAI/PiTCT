from distutils.core import Extension
from pathlib import Path

extensions = [
    Extension(
        "tctwrap.libtct",
        [str(cfile) for cfile in Path("tctwrap/").glob("**/*.c")],
    )
]


def build(setup_kwargs):
    """
    This function is mandatory in order to build the extensions.
    """
    setup_kwargs.update({"ext_modules": extensions})
