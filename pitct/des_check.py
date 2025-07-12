from logging import warning
from pathlib import Path
import warnings
from .config import Config

conf = Config.get_instance()

def gen_prm(filename: str, contents: str) -> str:
    prm = Path(conf.SAVE_FOLDER / filename)
    prm.write_text(contents)
    return str(prm)

def del_prm(filename: str):
    p = Path(conf.SAVE_FOLDER / filename)
    if p.exists():
        p.unlink()

def check_exist(filename: str):
    prm = Path(conf.SAVE_FOLDER / filename)
    if prm.exists():
        return True
    else:
        raise FileNotFoundError(f"File {filename} is not exists. Please create {filename}.")

def get_path(filename: str):
    """get file path

    create .DES or .DAT file path that consider save folder settings by init function.

    Args:
        filename (str): File path, including file extension.
    """
    f = Path(conf.SAVE_FOLDER / filename)
    return str(f)


def check_ret_code(ret_code: int):
    """check return code
    if return error code, this function raise Error.

    Args:
        retcode (int): Return code from c lang function
    """
    if ret_code == -1:
        raise FileExistsError("Error: Cannot open prm file. Please check if the prm file exists.")
    elif ret_code == -2:
        raise MemoryError("Error: Out of Memory.")
    elif ret_code == -3:
        raise RuntimeError("Error: Unexpected string found while loading the prm file. ")
    elif ret_code == -4:
        raise RuntimeError("Error: Supreduce internal error.")


def check_state_num(trans: list, size: int):
    max_state = 0
    for s, a, ns in trans:
        max_state = max(max_state, s, ns)
    max_state += 1
    
    if max_state == size:
        return
    elif max_state > size:
        raise RuntimeError(f"Error: number of state is too small. It must be set {max_state}")
    elif max_state < size:
        warnings.warn(f"Too many number of state. It is recommend to set {max_state}")