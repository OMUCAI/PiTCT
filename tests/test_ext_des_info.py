import pytest
from pitct.ext_des_info import ExtDesInfo
from pitct.name_converter import NameConverter


@pytest.fixture(autouse=True)
def reset_nc():
    NameConverter.reset()
    yield
    NameConverter.reset()


# reached=T, coreach=T, marked=T
# reached=T, coreach=F, marked=F
# reached=F, coreach=T, marked=F
EXT_DES = {
    0: {"reached": True, "coreach": True, "marked": True, "next": [[1, 1]], "vocal": 0},
    1: {"reached": True, "coreach": False, "marked": False, "next": None, "vocal": 0},
    2: {"reached": False, "coreach": True, "marked": False, "next": None, "vocal": 0},
}


@pytest.fixture
def ext_des():
    return ExtDesInfo("TEST", EXT_DES)


class TestExtDesInfoIsReached:
    def test_is_reached_true(self, ext_des):
        assert ext_des.is_reached(0) is True

    def test_is_reached_true_state1(self, ext_des):
        assert ext_des.is_reached(1) is True

    def test_is_reached_false(self, ext_des):
        assert ext_des.is_reached(2) is False

    def test_is_reached_out_of_index_raises(self, ext_des):
        with pytest.raises(RuntimeError):
            ext_des.is_reached(99)


class TestExtDesInfoAllReached:
    def test_all_reached_false_when_some_not(self, ext_des):
        assert ext_des.all_reached() is False

    def test_all_reached_true_when_all(self):
        all_reached = {
            0: {"reached": True, "coreach": True, "marked": True, "next": None, "vocal": 0},
            1: {"reached": True, "coreach": True, "marked": True, "next": None, "vocal": 0},
        }
        ext = ExtDesInfo("ALLR", all_reached)
        assert ext.all_reached() is True

    def test_all_reached_false_when_none(self):
        none_reached = {
            0: {"reached": False, "coreach": True, "marked": True, "next": None, "vocal": 0},
        }
        ext = ExtDesInfo("NONER", none_reached)
        assert ext.all_reached() is False


class TestExtDesInfoReachedList:
    def test_reached_list_contains_reached_states(self, ext_des):
        r = ext_des.reached(convert=False)
        assert 0 in r
        assert 1 in r

    def test_reached_list_excludes_not_reached(self, ext_des):
        r = ext_des.reached(convert=False)
        assert 2 not in r

    def test_reached_list_no_convert(self, ext_des):
        r = ext_des.reached(convert=False)
        assert all(isinstance(s, int) for s in r)


class TestExtDesInfoIsCoreach:
    def test_is_coreach_true(self, ext_des):
        assert ext_des.is_coreach(0) is True

    def test_is_coreach_false(self, ext_des):
        assert ext_des.is_coreach(1) is False

    def test_is_coreach_true_state2(self, ext_des):
        assert ext_des.is_coreach(2) is True

    def test_is_coreach_out_of_index_raises(self, ext_des):
        with pytest.raises(RuntimeError):
            ext_des.is_coreach(99)


class TestExtDesInfoAllCoreach:
    def test_all_coreach_false_when_some_not(self, ext_des):
        assert ext_des.all_coreach() is False

    def test_all_coreach_true_when_all(self):
        all_coreach = {
            0: {"reached": True, "coreach": True, "marked": True, "next": None, "vocal": 0},
            1: {"reached": True, "coreach": True, "marked": True, "next": None, "vocal": 0},
        }
        ext = ExtDesInfo("ALLC", all_coreach)
        assert ext.all_coreach() is True


class TestExtDesInfoCoreachList:
    def test_coreach_list_contains_coreach_states(self, ext_des):
        c = ext_des.coreach(convert=False)
        assert 0 in c
        assert 2 in c

    def test_coreach_list_excludes_not_coreach(self, ext_des):
        c = ext_des.coreach(convert=False)
        assert 1 not in c

    def test_coreach_no_convert(self, ext_des):
        c = ext_des.coreach(convert=False)
        assert all(isinstance(s, int) for s in c)


class TestExtDesInfoInheritance:
    def test_inherits_marked(self, ext_des):
        assert ext_des.is_marked(0) is True
        assert ext_des.is_marked(1) is False

    def test_inherits_next(self, ext_des):
        result = ext_des.next(0, convert=False)
        assert result is not None

    def test_inherits_events(self, ext_des):
        evts = ext_des.events(convert=False)
        assert 1 in evts
