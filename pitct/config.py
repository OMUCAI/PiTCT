from pathlib import Path
from typing import Any

DES_FILE_EXTENSION = ".DES"
EDES_FILE_EXTENSION = ".EDES"
DAT_FILE_EXTENSION = ".DAT"
RST_FILE_EXTENSION = ".RST"
TXT_FILE_EXTENSION = ".TXT"
class Singleton(object):
    @classmethod
    def get_instance(cls):
        if not hasattr(cls, "_instance"):
            cls._instance = cls()
        return cls._instance

class Config(Singleton):
    def __init__(self):
        self.SAVE_FOLDER = Path.cwd()
    
    def __getitem__(self, key):
        return self.__dict__[key]

    def  __setitem__(self, key, value):
        self.__dict__[key] = value

    def __setattr__(self, name: str, value: Any) -> None:
        self.__setitem__(name, value)
