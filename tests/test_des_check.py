import pytest
from pathlib import Path
from pitct.config import Config
from pitct.des_check import (
    gen_prm,
    del_prm,
    check_exist,
    get_path,
    check_ret_code,
    check_state_num,
)


@pytest.fixture
def temp_save_folder(tmp_path):
    conf = Config.get_instance()
    original = conf.SAVE_FOLDER
    conf.SAVE_FOLDER = tmp_path
    yield tmp_path
    conf.SAVE_FOLDER = original


class TestGenPrm:
    def test_creates_file(self, temp_save_folder):
        path = gen_prm("test.prm", "hello world")
        assert Path(path).exists()

    def test_file_contains_correct_content(self, temp_save_folder):
        path = gen_prm("test.prm", "my content")
        assert Path(path).read_text() == "my content"

    def test_returns_string_path(self, temp_save_folder):
        path = gen_prm("test.prm", "")
        assert isinstance(path, str)

    def test_file_in_save_folder(self, temp_save_folder):
        path = gen_prm("test.prm", "")
        assert str(temp_save_folder) in path


class TestDelPrm:
    def test_removes_existing_file(self, temp_save_folder):
        gen_prm("del_test.prm", "content")
        del_prm("del_test.prm")
        assert not (temp_save_folder / "del_test.prm").exists()

    def test_nonexistent_file_does_not_raise(self, temp_save_folder):
        del_prm("nonexistent.prm")  # should not raise


class TestCheckExist:
    def test_existing_file_returns_true(self, temp_save_folder):
        (temp_save_folder / "automaton.DES").write_text("data")
        assert check_exist("automaton.DES") is True

    def test_missing_file_raises_file_not_found(self, temp_save_folder):
        with pytest.raises(FileNotFoundError, match="not exists"):
            check_exist("missing.DES")


class TestGetPath:
    def test_returns_string(self, temp_save_folder):
        result = get_path("automaton.DES")
        assert isinstance(result, str)

    def test_contains_filename(self, temp_save_folder):
        result = get_path("automaton.DES")
        assert "automaton.DES" in result

    def test_contains_save_folder(self, temp_save_folder):
        result = get_path("plant.DES")
        assert str(temp_save_folder) in result


class TestCheckRetCode:
    def test_zero_does_not_raise(self):
        check_ret_code(0)

    def test_positive_does_not_raise(self):
        check_ret_code(1)
        check_ret_code(100)

    def test_minus1_raises_file_exists_error(self):
        with pytest.raises(FileExistsError):
            check_ret_code(-1)

    def test_minus2_raises_memory_error(self):
        with pytest.raises(MemoryError):
            check_ret_code(-2)

    def test_minus3_raises_runtime_error(self):
        with pytest.raises(RuntimeError, match="Unexpected string"):
            check_ret_code(-3)

    def test_minus4_raises_runtime_error(self):
        with pytest.raises(RuntimeError, match="internal error"):
            check_ret_code(-4)

    def test_other_negative_does_not_raise(self):
        # Only -1, -2, -3, -4 are handled; others silently pass
        check_ret_code(-5)


class TestCheckStateNum:
    def test_exact_state_count_ok(self):
        trans = [(0, "a", 1), (1, "b", 0)]
        check_state_num(trans, 2)  # max_state + 1 == 2 == size

    def test_size_too_small_raises(self):
        trans = [(0, "a", 2)]
        with pytest.raises(RuntimeError, match="too small"):
            check_state_num(trans, 2)  # max_state=2, max_state+1=3 > size=2

    def test_size_too_large_warns(self):
        trans = [(0, "a", 1)]
        with pytest.warns(UserWarning, match="Too many"):
            check_state_num(trans, 5)  # max_state+1=2 < size=5

    def test_single_state_loop(self):
        trans = [(0, "a", 0)]
        check_state_num(trans, 1)  # max_state=0, max_state+1=1 == size=1

    def test_state_0_only_with_size_1(self):
        trans = [(0, "a", 0), (0, "b", 0)]
        check_state_num(trans, 1)
