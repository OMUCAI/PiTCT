import pytest
from pitct.des_info import DesInfo
from pitct.name_converter import NameConverter


@pytest.fixture(autouse=True)
def reset_nc():
    NameConverter.reset()
    yield
    NameConverter.reset()


# State dict with integer states/events (no NameConverter encoding needed)
# 0: marked, transitions via event 1 -> state 1, event 3 -> state 2
# 1: marked, transitions via event 0 -> state 0
# 2: not marked, transitions via event 1 -> state 0
# 3: not marked, no transitions
SAMPLE_DES = {
    0: {"marked": True, "next": [[1, 1], [3, 2]], "vocal": 0},
    1: {"marked": True, "next": [[0, 0]], "vocal": 0},
    2: {"marked": False, "next": [[1, 0]], "vocal": 0},
    3: {"marked": False, "next": None, "vocal": 0},
}


@pytest.fixture
def des():
    return DesInfo("TEST", SAMPLE_DES)


class TestDesInfoNext:
    def test_next_from_state_0_has_two_entries(self, des):
        result = des.next(0, convert=False)
        assert result is not None
        assert len(result) == 2

    def test_next_from_state_0_contents(self, des):
        result = des.next(0, convert=False)
        assert [1, 1] in result   # event 1 -> state 1
        assert [3, 2] in result   # event 3 -> state 2

    def test_next_from_state_with_no_transitions_returns_none(self, des):
        result = des.next(3, convert=False)
        assert result is None

    def test_next_out_of_index_raises(self, des):
        with pytest.raises(RuntimeError):
            des.next(99, convert=False)

    def test_next_negative_index_raises(self, des):
        with pytest.raises(RuntimeError):
            des.next(-1, convert=False)


class TestDesInfoNextState:
    def test_next_state_found(self, des):
        # From state 0, event 1 -> state 1
        result = des.next_state(0, 1, convert=False)
        assert result == 1

    def test_next_state_another_event(self, des):
        # From state 0, event 3 -> state 2
        result = des.next_state(0, 3, convert=False)
        assert result == 2

    def test_next_state_undefined_event_returns_none(self, des):
        # Event 99 does not exist from state 0
        # We must register it first so event_encode can find it
        NameConverter.event_encode_dict[99] = "_dummy"
        result = des.next_state(0, 99, convert=False)
        assert result is None


class TestDesInfoMarked:
    def test_marked_returns_marked_states(self, des):
        marked = des.marked(convert=False)
        assert 0 in marked
        assert 1 in marked

    def test_marked_excludes_non_marked_states(self, des):
        marked = des.marked(convert=False)
        assert 2 not in marked
        assert 3 not in marked

    def test_is_marked_true_for_state_0(self, des):
        assert des.is_marked(0) is True

    def test_is_marked_true_for_state_1(self, des):
        assert des.is_marked(1) is True

    def test_is_marked_false_for_state_2(self, des):
        assert des.is_marked(2) is False

    def test_is_marked_out_of_index_raises(self, des):
        with pytest.raises(RuntimeError):
            des.is_marked(99)


class TestDesInfoTrans:
    def test_trans_count(self, des):
        # state 0: 2 transitions, state 1: 1, state 2: 1, state 3: None
        result = des.trans(convert=False)
        assert len(result) == 4

    def test_trans_excludes_none_states(self, des):
        result = des.trans(convert=False)
        # state 3 has None next, should not appear as source
        source_states = [t[0] for t in result]
        assert 3 not in source_states

    def test_trans_format_int_events(self, des):
        # With integer events, tuples have 3 elements (no c/u)
        result = des.trans(convert=False)
        for t in result:
            assert len(t) == 3

    def test_trans_format_string_events_has_cu(self):
        # When NameConverter has string event names, trans includes c/u
        NameConverter.state_encode_dict["S"] = {0: "idle", 1: "running"}
        NameConverter.event_encode_dict[1] = "go"
        NameConverter.event_encode_dict[0] = "stop"
        des_str = {
            0: {"marked": True, "next": [[1, 1]], "vocal": 0},
            1: {"marked": True, "next": [[0, 0]], "vocal": 0},
        }
        d = DesInfo("S", des_str)
        result = d.trans(convert=True)
        for t in result:
            assert len(t) == 4  # (state, event, next_state, c_or_u)


class TestDesInfoEvents:
    def test_events_returns_list(self, des):
        evts = des.events(convert=False)
        assert isinstance(evts, list)

    def test_events_contains_expected_values(self, des):
        evts = des.events(convert=False)
        assert set(evts) == {0, 1, 3}

    def test_events_no_duplicates(self, des):
        evts = des.events(convert=False)
        assert len(evts) == len(set(evts))


class TestDesInfoIterAndGetitem:
    def test_iter_yields_all_state_dicts(self, des):
        items = list(des)
        assert len(items) == 4

    def test_iter_yields_dicts_with_marked_key(self, des):
        for item in des:
            assert "marked" in item

    def test_getitem_state_0(self, des):
        item = des[0]
        assert item["marked"] is True
        assert item["next"] is not None

    def test_getitem_state_3(self, des):
        item = des[3]
        assert item["next"] is None

    def test_repr_returns_string(self, des):
        r = repr(des)
        assert isinstance(r, str)
