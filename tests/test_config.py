import pytest
from pathlib import Path
from pitct.config import (
    Config,
    DES_FILE_EXTENSION,
    EDES_FILE_EXTENSION,
    DAT_FILE_EXTENSION,
    RST_FILE_EXTENSION,
    TXT_FILE_EXTENSION,
)


class TestConfigSingleton:
    def test_same_instance(self):
        c1 = Config.get_instance()
        c2 = Config.get_instance()
        assert c1 is c2

    def test_default_save_folder_is_path(self):
        conf = Config.get_instance()
        assert isinstance(conf.SAVE_FOLDER, Path)


class TestConfigGetSetItem:
    def test_set_and_get_via_brackets(self):
        conf = Config.get_instance()
        original = conf.SAVE_FOLDER
        try:
            conf["SAVE_FOLDER"] = Path("/tmp")
            assert conf["SAVE_FOLDER"] == Path("/tmp")
        finally:
            conf.SAVE_FOLDER = original

    def test_set_attr_works(self):
        conf = Config.get_instance()
        original = conf.SAVE_FOLDER
        try:
            conf.SAVE_FOLDER = Path("/tmp/pitct_test")
            assert conf.SAVE_FOLDER == Path("/tmp/pitct_test")
        finally:
            conf.SAVE_FOLDER = original

    def test_set_and_get_custom_key(self):
        conf = Config.get_instance()
        conf["CUSTOM_KEY"] = "hello"
        assert conf["CUSTOM_KEY"] == "hello"
        del conf.__dict__["CUSTOM_KEY"]


class TestFileExtensions:
    def test_des_extension(self):
        assert DES_FILE_EXTENSION == ".DES"

    def test_edes_extension(self):
        assert EDES_FILE_EXTENSION == ".EDES"

    def test_dat_extension(self):
        assert DAT_FILE_EXTENSION == ".DAT"

    def test_rst_extension(self):
        assert RST_FILE_EXTENSION == ".RST"

    def test_txt_extension(self):
        assert TXT_FILE_EXTENSION == ".TXT"
